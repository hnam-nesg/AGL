#include "VieNeuTtsEngine.h"

#include <llama.h>
#include <onnxruntime_cxx_api.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace fs = std::filesystem;

namespace {
constexpr int kSampleRate = 24000;

#pragma pack(push, 1)
struct WavHeader {
    char riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t chunkSize = 0;
    char wave[4] = {'W', 'A', 'V', 'E'};
    char fmt[4] = {'f', 'm', 't', ' '};
    uint32_t fmtChunkSize = 16;
    uint16_t audioFormat = 1;
    uint16_t numChannels = 1;
    uint32_t sampleRate = kSampleRate;
    uint32_t byteRate = kSampleRate * 2;
    uint16_t blockAlign = 2;
    uint16_t bitsPerSample = 16;
    char data[4] = {'d', 'a', 't', 'a'};
    uint32_t dataSize = 0;
};
#pragma pack(pop)

struct LlamaBackendRef {
    LlamaBackendRef()
    {
        static std::once_flag once;
        std::call_once(once, []() {
            llama_backend_init();
        });
    }
};

void batchAdd(llama_batch& batch, llama_token id, int32_t pos, bool logits)
{
    const int i = batch.n_tokens;
    batch.token[i] = id;
    batch.pos[i] = pos;
    batch.n_seq_id[i] = 1;
    batch.seq_id[i][0] = 0;
    batch.logits[i] = logits;
    batch.n_tokens++;
}

size_t utf8CodepointCount(const std::string& text)
{
    size_t count = 0;
    for (unsigned char c : text) {
        if ((c & 0xc0) != 0x80) {
            ++count;
        }
    }
    return count;
}

template <typename T>
std::vector<T> parseNumberArray(const std::string& json, const std::string& key)
{
    const std::string marker = "\"" + key + "\"";
    const size_t keyPos = json.find(marker);
    if (keyPos == std::string::npos) {
        return {};
    }

    const size_t open = json.find('[', keyPos);
    if (open == std::string::npos) {
        return {};
    }

    const size_t close = json.find(']', open);
    if (close == std::string::npos) {
        return {};
    }

    std::vector<T> out;
    std::string body = json.substr(open + 1, close - open - 1);
    std::stringstream ss(body);
    std::string item;
    while (std::getline(ss, item, ',')) {
        try {
            size_t parsed = 0;
            const long long value = std::stoll(item, &parsed);
            while (parsed < item.size() &&
                   std::isspace(static_cast<unsigned char>(item[parsed]))) {
                ++parsed;
            }
            if (parsed != item.size()) {
                return {};
            }
            out.push_back(static_cast<T>(value));
        } catch (...) {
            return {};
        }
    }

    return out;
}

std::vector<float> parseFloatArray(const std::string& json, const std::string& key)
{
    const std::string marker = "\"" + key + "\"";
    const size_t keyPos = json.find(marker);
    if (keyPos == std::string::npos) {
        return {};
    }

    const size_t open = json.find('[', keyPos);
    if (open == std::string::npos) {
        return {};
    }

    const size_t close = json.find(']', open);
    if (close == std::string::npos) {
        return {};
    }

    std::vector<float> out;
    std::string body = json.substr(open + 1, close - open - 1);
    std::stringstream ss(body);
    std::string item;
    bool sawFloatSyntax = false;
    while (std::getline(ss, item, ',')) {
        try {
            if (item.find_first_of(".eE") != std::string::npos) {
                sawFloatSyntax = true;
            }
            size_t parsed = 0;
            const float value = std::stof(item, &parsed);
            while (parsed < item.size() &&
                   std::isspace(static_cast<unsigned char>(item[parsed]))) {
                ++parsed;
            }
            if (parsed != item.size()) {
                return {};
            }
            out.push_back(value);
        } catch (...) {
            return {};
        }
    }

    return sawFloatSyntax ? out : std::vector<float>{};
}

int hexValue(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

void appendUtf8(std::string& out, uint32_t cp)
{
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0x10FFFF) {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

bool decodeUtf8Codepoint(const std::string& text, size_t* pos, uint32_t* cp)
{
    if (!pos || !cp || *pos >= text.size()) {
        return false;
    }

    const unsigned char c0 = static_cast<unsigned char>(text[*pos]);
    if (c0 < 0x80) {
        *cp = c0;
        ++(*pos);
        return true;
    }

    int needed = 0;
    uint32_t value = 0;
    uint32_t minValue = 0;
    if ((c0 & 0xE0) == 0xC0) {
        needed = 1;
        value = c0 & 0x1F;
        minValue = 0x80;
    } else if ((c0 & 0xF0) == 0xE0) {
        needed = 2;
        value = c0 & 0x0F;
        minValue = 0x800;
    } else if ((c0 & 0xF8) == 0xF0) {
        needed = 3;
        value = c0 & 0x07;
        minValue = 0x10000;
    } else {
        *cp = c0;
        ++(*pos);
        return false;
    }

    if (*pos + static_cast<size_t>(needed) >= text.size()) {
        *cp = c0;
        ++(*pos);
        return false;
    }

    for (int i = 1; i <= needed; ++i) {
        const unsigned char cx = static_cast<unsigned char>(text[*pos + static_cast<size_t>(i)]);
        if ((cx & 0xC0) != 0x80) {
            *cp = c0;
            ++(*pos);
            return false;
        }
        value = (value << 6) | static_cast<uint32_t>(cx & 0x3F);
    }

    if (value < minValue || value > 0x10FFFF ||
        (value >= 0xD800 && value <= 0xDFFF)) {
        *cp = c0;
        ++(*pos);
        return false;
    }

    *cp = value;
    *pos += static_cast<size_t>(needed + 1);
    return true;
}

uint32_t lowerVietnameseCodepoint(uint32_t cp)
{
    if (cp >= 'A' && cp <= 'Z') {
        return cp + ('a' - 'A');
    }

    switch (cp) {
    case 0x00C0: return 0x00E0; // À
    case 0x00C1: return 0x00E1; // Á
    case 0x1EA2: return 0x1EA3; // Ả
    case 0x00C3: return 0x00E3; // Ã
    case 0x1EA0: return 0x1EA1; // Ạ
    case 0x0102: return 0x0103; // Ă
    case 0x1EB0: return 0x1EB1; // Ằ
    case 0x1EAE: return 0x1EAF; // Ắ
    case 0x1EB2: return 0x1EB3; // Ẳ
    case 0x1EB4: return 0x1EB5; // Ẵ
    case 0x1EB6: return 0x1EB7; // Ặ
    case 0x00C2: return 0x00E2; // Â
    case 0x1EA6: return 0x1EA7; // Ầ
    case 0x1EA4: return 0x1EA5; // Ấ
    case 0x1EA8: return 0x1EA9; // Ẩ
    case 0x1EAA: return 0x1EAB; // Ẫ
    case 0x1EAC: return 0x1EAD; // Ậ
    case 0x0110: return 0x0111; // Đ
    case 0x00C8: return 0x00E8; // È
    case 0x00C9: return 0x00E9; // É
    case 0x1EBA: return 0x1EBB; // Ẻ
    case 0x1EBC: return 0x1EBD; // Ẽ
    case 0x1EB8: return 0x1EB9; // Ẹ
    case 0x00CA: return 0x00EA; // Ê
    case 0x1EC0: return 0x1EC1; // Ề
    case 0x1EBE: return 0x1EBF; // Ế
    case 0x1EC2: return 0x1EC3; // Ể
    case 0x1EC4: return 0x1EC5; // Ễ
    case 0x1EC6: return 0x1EC7; // Ệ
    case 0x00CC: return 0x00EC; // Ì
    case 0x00CD: return 0x00ED; // Í
    case 0x1EC8: return 0x1EC9; // Ỉ
    case 0x0128: return 0x0129; // Ĩ
    case 0x1ECA: return 0x1ECB; // Ị
    case 0x00D2: return 0x00F2; // Ò
    case 0x00D3: return 0x00F3; // Ó
    case 0x1ECE: return 0x1ECF; // Ỏ
    case 0x00D5: return 0x00F5; // Õ
    case 0x1ECC: return 0x1ECD; // Ọ
    case 0x00D4: return 0x00F4; // Ô
    case 0x1ED2: return 0x1ED3; // Ồ
    case 0x1ED0: return 0x1ED1; // Ố
    case 0x1ED4: return 0x1ED5; // Ổ
    case 0x1ED6: return 0x1ED7; // Ỗ
    case 0x1ED8: return 0x1ED9; // Ộ
    case 0x01A0: return 0x01A1; // Ơ
    case 0x1EDC: return 0x1EDD; // Ờ
    case 0x1EDA: return 0x1EDB; // Ớ
    case 0x1EDE: return 0x1EDF; // Ở
    case 0x1EE0: return 0x1EE1; // Ỡ
    case 0x1EE2: return 0x1EE3; // Ợ
    case 0x00D9: return 0x00F9; // Ù
    case 0x00DA: return 0x00FA; // Ú
    case 0x1EE6: return 0x1EE7; // Ủ
    case 0x0168: return 0x0169; // Ũ
    case 0x1EE4: return 0x1EE5; // Ụ
    case 0x01AF: return 0x01B0; // Ư
    case 0x1EEA: return 0x1EEB; // Ừ
    case 0x1EE8: return 0x1EE9; // Ứ
    case 0x1EEC: return 0x1EED; // Ử
    case 0x1EEE: return 0x1EEF; // Ữ
    case 0x1EF0: return 0x1EF1; // Ự
    case 0x1EF2: return 0x1EF3; // Ỳ
    case 0x00DD: return 0x00FD; // Ý
    case 0x1EF6: return 0x1EF7; // Ỷ
    case 0x1EF8: return 0x1EF9; // Ỹ
    case 0x1EF4: return 0x1EF5; // Ỵ
    default:
        return cp;
    }
}

std::string lowerVietnameseUtf8(const std::string& text)
{
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size();) {
        const size_t before = i;
        uint32_t cp = 0;
        if (!decodeUtf8Codepoint(text, &i, &cp)) {
            const unsigned char byte = static_cast<unsigned char>(text[before]);
            out.push_back(static_cast<char>(std::tolower(byte)));
            continue;
        }
        appendUtf8(out, lowerVietnameseCodepoint(cp));
    }
    return out;
}

bool isAsciiAlpha(char c)
{
    const unsigned char uc = static_cast<unsigned char>(c);
    return (uc >= 'A' && uc <= 'Z') || (uc >= 'a' && uc <= 'z');
}

bool isAsciiUpperAlpha(char c)
{
    const unsigned char uc = static_cast<unsigned char>(c);
    return uc >= 'A' && uc <= 'Z';
}

bool isAsciiAlnum(char c)
{
    const unsigned char uc = static_cast<unsigned char>(c);
    return (uc >= 'A' && uc <= 'Z') ||
           (uc >= 'a' && uc <= 'z') ||
           (uc >= '0' && uc <= '9');
}

const char* vietnameseDigitWord(char c)
{
    switch (c) {
    case '0': return "không";
    case '1': return "một";
    case '2': return "hai";
    case '3': return "ba";
    case '4': return "bốn";
    case '5': return "năm";
    case '6': return "sáu";
    case '7': return "bảy";
    case '8': return "tám";
    case '9': return "chín";
    default: return "";
    }
}

const char* englishLetterPhoneme(char c)
{
    switch (static_cast<char>(std::toupper(static_cast<unsigned char>(c)))) {
    case 'A': return "ˈeɪ";
    case 'B': return "bˈiː";
    case 'C': return "sˈiː";
    case 'D': return "dˈiː";
    case 'E': return "ˈiː";
    case 'F': return "ˈɛf";
    case 'G': return "dʒˈiː";
    case 'H': return "ˈeɪtʃ";
    case 'I': return "ˈaɪ";
    case 'J': return "dʒˈeɪ";
    case 'K': return "kˈeɪ";
    case 'L': return "ˈɛl";
    case 'M': return "ˈɛm";
    case 'N': return "ˈɛn";
    case 'O': return "ˈoʊ";
    case 'P': return "pˈiː";
    case 'Q': return "kjˈuː";
    case 'R': return "ˈɑːɹ";
    case 'S': return "ˈɛs";
    case 'T': return "tˈiː";
    case 'U': return "jˈuː";
    case 'V': return "vˈiː";
    case 'W': return "dˈʌbəljˌuː";
    case 'X': return "ˈɛks";
    case 'Y': return "wˈaɪ";
    case 'Z': return "zˈiː";
    default: return "";
    }
}

void skipJsonWhitespace(const std::string& json, size_t* pos)
{
    while (pos && *pos < json.size() &&
           std::isspace(static_cast<unsigned char>(json[*pos]))) {
        ++(*pos);
    }
}

bool parseJsonStringAt(const std::string& json, size_t* pos, std::string* out)
{
    if (!pos || !out || *pos >= json.size() || json[*pos] != '"') {
        return false;
    }

    out->clear();
    bool escape = false;
    for (size_t i = *pos + 1; i < json.size(); ++i) {
        const char c = json[i];
        if (escape) {
            switch (c) {
            case '"':
            case '\\':
            case '/':
                out->push_back(c);
                break;
            case 'b':
                out->push_back('\b');
                break;
            case 'f':
                out->push_back('\f');
                break;
            case 'n':
                out->push_back('\n');
                break;
            case 'r':
                out->push_back('\r');
                break;
            case 't':
                out->push_back('\t');
                break;
            case 'u': {
                if (i + 4 >= json.size()) {
                    out->push_back(c);
                    break;
                }

                uint32_t value = 0;
                bool valid = true;
                for (size_t j = 1; j <= 4; ++j) {
                    const int v = hexValue(json[i + j]);
                    if (v < 0) {
                        valid = false;
                        break;
                    }
                    value = (value << 4) | static_cast<uint32_t>(v);
                }

                if (valid) {
                    if (value >= 0xD800 && value <= 0xDBFF &&
                        i + 10 < json.size() &&
                        json[i + 5] == '\\' &&
                        json[i + 6] == 'u') {
                        uint32_t low = 0;
                        bool lowValid = true;
                        for (size_t j = 7; j <= 10; ++j) {
                            const int v = hexValue(json[i + j]);
                            if (v < 0) {
                                lowValid = false;
                                break;
                            }
                            low = (low << 4) | static_cast<uint32_t>(v);
                        }

                        if (lowValid && low >= 0xDC00 && low <= 0xDFFF) {
                            value = 0x10000 + (((value - 0xD800) << 10) | (low - 0xDC00));
                            i += 6;
                        }
                    }

                    appendUtf8(*out, value);
                    i += 4;
                } else {
                    out->push_back(c);
                }
                break;
            }
            default:
                out->push_back(c);
                break;
            }
            escape = false;
            continue;
        }

        if (c == '\\') {
            escape = true;
            continue;
        }
        if (c == '"') {
            *pos = i + 1;
            return true;
        }
        out->push_back(c);
    }

    return false;
}

std::unordered_map<std::string, std::string> parseStringMap(const std::string& json)
{
    std::unordered_map<std::string, std::string> out;
    out.reserve(std::max<size_t>(1024, json.size() / 48));

    size_t pos = 0;
    skipJsonWhitespace(json, &pos);
    if (pos >= json.size() || json[pos] != '{') {
        return {};
    }
    ++pos;

    while (pos < json.size()) {
        skipJsonWhitespace(json, &pos);
        if (pos < json.size() && json[pos] == '}') {
            ++pos;
            return out;
        }

        std::string key;
        std::string value;
        if (!parseJsonStringAt(json, &pos, &key)) {
            return {};
        }

        skipJsonWhitespace(json, &pos);
        if (pos >= json.size() || json[pos] != ':') {
            return {};
        }
        ++pos;
        skipJsonWhitespace(json, &pos);

        if (!parseJsonStringAt(json, &pos, &value)) {
            return {};
        }
        out.emplace(std::move(key), std::move(value));

        skipJsonWhitespace(json, &pos);
        if (pos < json.size() && json[pos] == ',') {
            ++pos;
            continue;
        }
        if (pos < json.size() && json[pos] == '}') {
            ++pos;
            return out;
        }
        return {};
    }

    return {};
}

std::string parseStringValue(const std::string& json, const std::string& key)
{
    const std::string marker = "\"" + key + "\"";
    const size_t keyPos = json.find(marker);
    if (keyPos == std::string::npos) {
        return {};
    }

    const size_t colon = json.find(':', keyPos);
    if (colon == std::string::npos) {
        return {};
    }

    const size_t open = json.find('"', colon + 1);
    if (open == std::string::npos) {
        return {};
    }

    std::string out;
    bool escape = false;
    for (size_t i = open + 1; i < json.size(); ++i) {
        const char c = json[i];
        if (escape) {
            switch (c) {
            case '"':
            case '\\':
            case '/':
                out.push_back(c);
                break;
            case 'b':
                out.push_back('\b');
                break;
            case 'f':
                out.push_back('\f');
                break;
            case 'n':
                out.push_back('\n');
                break;
            case 'r':
                out.push_back('\r');
                break;
            case 't':
                out.push_back('\t');
                break;
            case 'u': {
                if (i + 4 >= json.size()) {
                    out.push_back(c);
                    break;
                }

                uint32_t cp = 0;
                bool valid = true;
                for (size_t j = 1; j <= 4; ++j) {
                    const int v = hexValue(json[i + j]);
                    if (v < 0) {
                        valid = false;
                        break;
                    }
                    cp = (cp << 4) | static_cast<uint32_t>(v);
                }

                if (valid) {
                    if (cp >= 0xD800 && cp <= 0xDBFF &&
                        i + 10 < json.size() &&
                        json[i + 5] == '\\' &&
                        json[i + 6] == 'u') {
                        uint32_t low = 0;
                        bool lowValid = true;
                        for (size_t j = 7; j <= 10; ++j) {
                            const int v = hexValue(json[i + j]);
                            if (v < 0) {
                                lowValid = false;
                                break;
                            }
                            low = (low << 4) | static_cast<uint32_t>(v);
                        }

                        if (lowValid && low >= 0xDC00 && low <= 0xDFFF) {
                            cp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));
                            i += 6;
                        }
                    }

                    appendUtf8(out, cp);
                    i += 4;
                } else {
                    out.push_back(c);
                }
                break;
            }
            default:
                out.push_back(c);
                break;
            }
            escape = false;
            continue;
        }
        if (c == '\\') {
            escape = true;
            continue;
        }
        if (c == '"') {
            break;
        }
        out.push_back(c);
    }

    return out;
}

std::string findObjectValueForKey(const std::string& json, const std::string& key)
{
    const std::string marker = "\"" + key + "\"";
    size_t searchPos = 0;

    while (true) {
        const size_t keyPos = json.find(marker, searchPos);
        if (keyPos == std::string::npos) {
            return {};
        }

        const size_t colon = json.find(':', keyPos + marker.size());
        if (colon == std::string::npos) {
            return {};
        }

        size_t open = colon + 1;
        while (open < json.size() &&
               std::isspace(static_cast<unsigned char>(json[open]))) {
            ++open;
        }

        if (open >= json.size() || json[open] != '{') {
            searchPos = keyPos + marker.size();
            continue;
        }

        bool inString = false;
        bool escape = false;
        int depth = 0;

        for (size_t i = open; i < json.size(); ++i) {
            const char c = json[i];
            if (escape) {
                escape = false;
                continue;
            }
            if (c == '\\') {
                escape = inString;
                continue;
            }
            if (c == '"') {
                inString = !inString;
                continue;
            }
            if (inString) {
                continue;
            }
            if (c == '{') {
                ++depth;
                continue;
            }
            if (c == '}') {
                --depth;
                if (depth == 0) {
                    return json.substr(open, i - open + 1);
                }
            }
        }

        return {};
    }
}

bool pathExists(const fs::path& path)
{
    std::error_code ec;
    return fs::is_regular_file(path, ec) || fs::exists(path, ec);
}

std::string resolveCodecPath(const std::string& configured)
{
    const fs::path configuredPath(configured);
    if (!configured.empty() && pathExists(configuredPath)) {
        return configured;
    }
    return {};
}

bool equalsIgnoreCase(std::string_view lhs, std::string_view rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (size_t i = 0; i < lhs.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(lhs[i])) !=
            std::tolower(static_cast<unsigned char>(rhs[i]))) {
            return false;
        }
    }
    return true;
}

std::string trim(std::string text)
{
    while (!text.empty() &&
           std::isspace(static_cast<unsigned char>(text.front()))) {
        text.erase(text.begin());
    }
    while (!text.empty() &&
           std::isspace(static_cast<unsigned char>(text.back()))) {
        text.pop_back();
    }
    return text;
}

struct SynthesisStateGuard {
    std::atomic<bool>* synthesizing = nullptr;

    ~SynthesisStateGuard()
    {
        if (synthesizing) {
            synthesizing->store(false);
        }
    }
};

struct LlamaSamplerDeleter {
    void operator()(llama_sampler* sampler) const
    {
        if (sampler) {
            llama_sampler_free(sampler);
        }
    }
};

class LlamaBatchGuard {
public:
    explicit LlamaBatchGuard(int32_t n_tokens)
        : batch_(llama_batch_init(n_tokens, 0, 1))
    {
    }

    ~LlamaBatchGuard()
    {
        llama_batch_free(batch_);
    }

    LlamaBatchGuard(const LlamaBatchGuard&) = delete;
    LlamaBatchGuard& operator=(const LlamaBatchGuard&) = delete;

    llama_batch& get()
    {
        return batch_;
    }

    bool valid() const
    {
        return batch_.token && batch_.pos && batch_.n_seq_id && batch_.seq_id && batch_.logits;
    }

private:
    llama_batch batch_{};
};
} // namespace

VieNeuTtsEngine::VieNeuTtsEngine(const Config& cfg)
    : cfg_(cfg)
{
}

VieNeuTtsEngine::~VieNeuTtsEngine()
{
    stop();

    if (ctx_) {
        llama_free(ctx_);
        ctx_ = nullptr;
    }
    if (model_) {
        llama_model_free(model_);
        model_ = nullptr;
    }
}

bool VieNeuTtsEngine::initialize()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_) {
        return true;
    }

    LlamaBackendRef backend;

    if (!loadVoice() || !loadCodec() || !loadPhonemeDictionary()) {
        return false;
    }

    const PromptMode mode = activePromptMode();
    if (mode == PromptMode::Standard && voice_.codes.empty()) {
        std::cerr << "[VieNeuTTS] Standard prompt requires integer voice codes. "
                  << "Use a standard voices.json or set prompt_mode=turbo with a v2 model.\n";
        return false;
    }
    if (mode == PromptMode::Turbo && voice_.embedding.empty()) {
        std::cerr << "[VieNeuTTS] Turbo prompt requires a 128-float voice embedding. "
                  << "Use a v2/Turbo voices.json or set prompt_mode=standard with a legacy codec.\n";
        return false;
    }
    if (!loadBackbone()) {
        return false;
    }

    initialized_ = true;

    if (cfg_.warmup) {
        warmup();
    }

    return true;
}

bool VieNeuTtsEngine::loadBackbone()
{
    if (!fs::exists(cfg_.model_path)) {
        std::cerr << "[VieNeuTTS] GGUF model not found: "
                  << cfg_.model_path << "\n";
        return false;
    }

    llama_model_params modelParams = llama_model_default_params();
    modelParams.n_gpu_layers = cfg_.n_gpu_layers;
    modelParams.use_mmap = cfg_.use_mmap;
    modelParams.use_mlock = cfg_.use_mlock;

    model_ = llama_model_load_from_file(cfg_.model_path.c_str(), modelParams);
    if (!model_) {
        std::cerr << "[VieNeuTTS] Failed to load GGUF model: "
                  << cfg_.model_path << "\n";
        return false;
    }

    llama_context_params ctxParams = llama_context_default_params();
    const int batch = std::max(32, cfg_.batch);
    ctxParams.n_ctx = static_cast<uint32_t>(std::max(512, cfg_.ctx));
    ctxParams.n_batch = static_cast<uint32_t>(batch);
    ctxParams.n_ubatch = static_cast<uint32_t>(
        std::max(1, std::min(std::max(1, cfg_.ubatch), batch)));
    const int threads = std::max(1, cfg_.threads);
    ctxParams.n_threads = threads;
    ctxParams.n_threads_batch = cfg_.batch_threads > 0 ? cfg_.batch_threads : threads;
    ctxParams.no_perf = true;

    ctx_ = llama_init_from_model(model_, ctxParams);
    if (!ctx_) {
        std::cerr << "[VieNeuTTS] Failed to create llama context\n";
        return false;
    }

    vocab_ = llama_model_get_vocab(model_);
    if (!vocab_) {
        std::cerr << "[VieNeuTTS] Failed to get llama vocab\n";
        return false;
    }

    return true;
}

bool VieNeuTtsEngine::loadCodec()
{
    const std::string codecPath = resolveCodecPath(cfg_.codec_path);
    if (codecPath.empty()) {
        std::cerr << "[VieNeuTTS] codec ONNX not found: "
                  << cfg_.codec_path
                  << ". For VieNeu-TTS-0.3B Q4 standard mode, copy the legacy "
                  << "neuphonic/neucodec-onnx-decoder-int8 model.onnx here. "
                  << "Do not use VieNeu-Codec v2; it requires voice_embedding.\n";
        return false;
    }

    try {
        ort_env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "VieNeuTTS");
        ort_session_options_ = std::make_unique<Ort::SessionOptions>();
        ort_session_options_->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        ort_session_options_->SetIntraOpNumThreads(std::max(1, cfg_.codec_threads));
        ort_session_ = std::make_unique<Ort::Session>(
            *ort_env_,
            codecPath.c_str(),
            *ort_session_options_);

        Ort::AllocatorWithDefaultOptions allocator;
        auto outputName = ort_session_->GetOutputNameAllocated(0, allocator);
        codec_output_name_ = outputName.get();

        codec_content_input_name_.clear();
        codec_voice_input_name_.clear();
        codec_content_input_element_type_ = 0;
        codec_content_input_rank_ = 0;
        codec_voice_embedding_size_ = 0;
        codec_uses_voice_embedding_ = false;

        const size_t inputCount = ort_session_->GetInputCount();
        for (size_t i = 0; i < inputCount; ++i) {
            auto inputName = ort_session_->GetInputNameAllocated(i, allocator);
            const std::string name = inputName.get();
            const auto inputTypeInfo = ort_session_->GetInputTypeInfo(i);
            const auto inputTensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();
            const int elementType = static_cast<int>(inputTensorInfo.GetElementType());
            const std::vector<int64_t> shape = inputTensorInfo.GetShape();

            if (name == "content_ids" ||
                (codec_content_input_name_.empty() &&
                 (elementType == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32 ||
                  elementType == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64))) {
                codec_content_input_name_ = name;
                codec_content_input_element_type_ = elementType;
                codec_content_input_rank_ = shape.size();
            }

            if (name == "voice_embedding" || name == "voice") {
                if (elementType != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
                    std::cerr << "[VieNeuTTS] Unsupported codec voice input dtype: "
                              << elementType
                              << " expected float\n";
                    return false;
                }

                codec_voice_input_name_ = name;
                codec_uses_voice_embedding_ = true;
                if (!shape.empty() && shape.back() > 0) {
                    codec_voice_embedding_size_ = static_cast<size_t>(shape.back());
                }
            }
        }

        if (codec_content_input_name_.empty()) {
            std::cerr << "[VieNeuTTS] Codec content_ids input not found\n";
            return false;
        }
        if (codec_content_input_element_type_ != ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32 &&
            codec_content_input_element_type_ != ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
            std::cerr << "[VieNeuTTS] Unsupported codec content_ids dtype: "
                      << codec_content_input_element_type_
                      << " expected int32 or int64\n";
            return false;
        }
        if (codec_uses_voice_embedding_) {
            const size_t expected = codec_voice_embedding_size_ == 0
                ? static_cast<size_t>(128)
                : codec_voice_embedding_size_;
            if (voice_.embedding.size() != expected) {
                std::cerr << "[VieNeuTTS] Codec requires "
                          << expected
                          << "-float voice_embedding, but voice preset has "
                          << voice_.embedding.size()
                          << " floats. Use a VieNeu v2/Turbo voices.json with this codec, "
                          << "or configure a legacy NeuCodec decoder for code presets.\n";
                return false;
            }
        }

        std::cout << "[VieNeuTTS] codec content input="
                  << codec_content_input_name_
                  << " dtype="
                  << (codec_content_input_element_type_ == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64
                          ? "int64"
                          : "int32")
                  << " rank="
                  << codec_content_input_rank_;
        if (codec_uses_voice_embedding_) {
            std::cout << " voice_input="
                      << codec_voice_input_name_
                      << " floats="
                      << voice_.embedding.size();
        }
        std::cout << " path="
                  << codecPath
                  << "\n";
    } catch (const std::exception& e) {
        std::cerr << "[VieNeuTTS] Failed to load ONNX codec: "
                  << e.what() << "\n";
        return false;
    }

    const PromptMode requestedMode = configuredPromptMode();
    if (requestedMode == PromptMode::Standard && codec_uses_voice_embedding_) {
        std::cerr << "[VieNeuTTS] prompt_mode=standard is for VieNeu-TTS-0.3B Q4 and "
                  << "requires a legacy one-input NeuCodec decoder. The configured codec has "
                  << codec_voice_input_name_
                  << ", so it is VieNeu-Codec v2 and is not compatible with voice code preset "
                  << cfg_.voice
                  << ". Use /opt/vieneu-gguf/models/neucodec/model.onnx, not "
                  << "/opt/vieneu-gguf/models/vieneu-q4/model.onnx or VieNeu-Codec v2.\n";
        return false;
    }
    if (requestedMode == PromptMode::Turbo && !codec_uses_voice_embedding_) {
        std::cerr << "[VieNeuTTS] prompt_mode=turbo requires a codec with voice_embedding input\n";
        return false;
    }

    return true;
}

bool VieNeuTtsEngine::loadVoice()
{
    if (!fs::exists(cfg_.voices_path)) {
        std::cerr << "[VieNeuTTS] voices.json not found: "
                  << cfg_.voices_path << "\n";
        return false;
    }

    const std::string json = readFile(cfg_.voices_path);
    if (json.empty()) {
        std::cerr << "[VieNeuTTS] voices.json is empty\n";
        return false;
    }

    const std::string wanted = cfg_.voice.empty()
        ? parseStringValue(json, "default_voice")
        : cfg_.voice;

    const std::string voiceBlock = findObjectValueForKey(json, wanted);
    if (voiceBlock.empty()) {
        std::cerr << "[VieNeuTTS] Voice not found in voices.json: "
                  << wanted << "\n";
        return false;
    }

    voice_.codes = parseNumberArray<int32_t>(voiceBlock, "codes");
    voice_.embedding = parseFloatArray(voiceBlock, "voice_embedding");
    if (voice_.embedding.empty()) {
        voice_.embedding = parseFloatArray(voiceBlock, "embedding");
    }
    if (voice_.embedding.empty()) {
        voice_.embedding = parseFloatArray(voiceBlock, "codes");
    }
    voice_.text = parseStringValue(voiceBlock, "text");

    if (voice_.codes.empty() && voice_.embedding.empty()) {
        std::cerr << "[VieNeuTTS] Invalid voice preset: " << wanted << "\n";
        return false;
    }

    std::cout << "[VieNeuTTS] Loaded voice " << wanted
              << " codes=" << voice_.codes.size()
              << " embedding=" << voice_.embedding.size()
              << "\n";
    return true;
}

bool VieNeuTtsEngine::setVoice(const std::string& voice)
{
    if (voice.empty()) {
        return true;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (cfg_.voice == voice) {
        return true;
    }

    const std::string previousVoiceName = cfg_.voice;
    const VoicePreset previousVoice = voice_;

    cfg_.voice = voice;
    if (!loadVoice()) {
        cfg_.voice = previousVoiceName;
        voice_ = previousVoice;
        return false;
    }

    const PromptMode mode = activePromptMode();
    if ((mode == PromptMode::Standard && voice_.codes.empty()) ||
        (mode == PromptMode::Turbo && voice_.embedding.empty())) {
        std::cerr << "[VieNeuTTS] Voice preset incompatible with current prompt mode: "
                  << voice
                  << "\n";
        cfg_.voice = previousVoiceName;
        voice_ = previousVoice;
        return false;
    }

    if (codec_uses_voice_embedding_) {
        const size_t expected = codec_voice_embedding_size_ == 0
            ? static_cast<size_t>(128)
            : codec_voice_embedding_size_;
        if (voice_.embedding.size() != expected) {
            std::cerr << "[VieNeuTTS] Voice preset "
                      << voice
                      << " has "
                      << voice_.embedding.size()
                      << " embedding floats, expected "
                      << expected
                      << "\n";
            cfg_.voice = previousVoiceName;
            voice_ = previousVoice;
            return false;
        }
    }

    return true;
}

bool VieNeuTtsEngine::loadPhonemeDictionary()
{
    phoneme_dict_.clear();

    const PromptMode mode = activePromptMode();
    if (mode == PromptMode::Turbo) {
        return true;
    }

    if (cfg_.phoneme_dict_path.empty() || !fs::exists(cfg_.phoneme_dict_path)) {
        if (cfg_.require_phoneme_dict) {
            std::cerr << "[VieNeuTTS] phoneme_dict.json not found: "
                      << cfg_.phoneme_dict_path
                      << ". VieNeu-TTS-0.3B Q4 needs phonemized text; raw text often generates only a few speech tokens.\n";
            return false;
        }

        std::cerr << "[VieNeuTTS] phoneme_dict.json not found: "
                  << cfg_.phoneme_dict_path
                  << ". Falling back to raw normalized text.\n";
        return true;
    }

    const std::string json = readFile(cfg_.phoneme_dict_path);
    if (json.empty()) {
        std::cerr << "[VieNeuTTS] phoneme_dict.json is empty: "
                  << cfg_.phoneme_dict_path
                  << "\n";
        return !cfg_.require_phoneme_dict;
    }

    phoneme_dict_ = parseStringMap(json);
    if (phoneme_dict_.empty()) {
        std::cerr << "[VieNeuTTS] Failed to parse phoneme_dict.json: "
                  << cfg_.phoneme_dict_path
                  << "\n";
        return !cfg_.require_phoneme_dict;
    }

    std::cout << "[VieNeuTTS] Loaded phoneme dictionary entries="
              << phoneme_dict_.size()
              << " path="
              << cfg_.phoneme_dict_path
              << "\n";
    return true;
}

bool VieNeuTtsEngine::warmup()
{
    const bool old = stop_requested_.load();
    stop_requested_.store(false);

    try {
        const std::string prompt = buildPrompt(cfg_.warmup_text);
        if (prompt.empty()) {
            stop_requested_.store(old);
            return false;
        }
        (void)generateSpeechTokens(prompt, 8);
        if (activePromptMode() == PromptMode::Standard && !voice_.codes.empty()) {
            const size_t n = std::min<size_t>(voice_.codes.size(), static_cast<size_t>(16));
            std::vector<int32_t> codecWarmup;
            codecWarmup.assign(voice_.codes.begin(), voice_.codes.begin() + n);
            (void)decodeSpeechIds(codecWarmup);
        }
    } catch (...) {
        stop_requested_.store(old);
        return false;
    }

    stop_requested_.store(old);
    return true;
}

bool VieNeuTtsEngine::isReady() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

bool VieNeuTtsEngine::isSynthesizing() const
{
    return synthesizing_.load();
}

void VieNeuTtsEngine::resetStopRequested()
{
    stop_requested_.store(false);
}

void VieNeuTtsEngine::stop()
{
    stop_requested_.store(true);
}

std::vector<int32_t> VieNeuTtsEngine::tokenize(const std::string& text,
                                               bool addSpecial,
                                               bool parseSpecial) const
{
    const int needed = -llama_tokenize(
        vocab_,
        text.c_str(),
        static_cast<int32_t>(text.size()),
        nullptr,
        0,
        addSpecial,
        parseSpecial);

    if (needed <= 0) {
        return {};
    }

    std::vector<llama_token> raw(static_cast<size_t>(needed));
    const int actual = llama_tokenize(
        vocab_,
        text.c_str(),
        static_cast<int32_t>(text.size()),
        raw.data(),
        static_cast<int32_t>(raw.size()),
        addSpecial,
        parseSpecial);

    if (actual < 0) {
        return {};
    }

    return std::vector<int32_t>(raw.begin(), raw.begin() + actual);
}

std::string VieNeuTtsEngine::tokenToPiece(int32_t token, bool special) const
{
    char buffer[256];
    int n = llama_token_to_piece(
        vocab_,
        token,
        buffer,
        sizeof(buffer),
        0,
        special);

    if (n == 0) {
        return {};
    }

    if (n < 0) {
        std::vector<char> dynamicBuffer(static_cast<size_t>(-n));
        n = llama_token_to_piece(
            vocab_,
            token,
            dynamicBuffer.data(),
            static_cast<int32_t>(dynamicBuffer.size()),
            0,
            special);

        if (n <= 0) {
            return {};
        }

        return std::string(dynamicBuffer.data(), static_cast<size_t>(n));
    }

    return std::string(buffer, static_cast<size_t>(n));
}

int32_t VieNeuTtsEngine::tokenForPiece(const std::string& text) const
{
    const std::vector<int32_t> tokens = tokenize(text, false, true);
    if (tokens.size() != 1) {
        return -1;
    }
    return tokens.front();
}

std::string VieNeuTtsEngine::generateSpeechTokens(const std::string& prompt, int max_tokens)
{
    llama_memory_clear(llama_get_memory(ctx_), true);

    const int32_t speechStartId = tokenForPiece("<|SPEECH_GENERATION_START|>");
    const int32_t speechEndId = tokenForPiece("<|SPEECH_GENERATION_END|>");
    const int32_t textStartId = tokenForPiece("<|TEXT_PROMPT_START|>");
    const int32_t textEndId = tokenForPiece("<|TEXT_PROMPT_END|>");
    if (speechStartId < 0 || speechEndId < 0 || textStartId < 0 || textEndId < 0) {
        std::cerr << "[VieNeuTTS] GGUF vocab is missing VieNeu special tokens. "
                  << "Check that model_path points to VieNeu-TTS-0_3B-Q4_0.gguf.\n";
        return {};
    }

    std::vector<int32_t> promptTokens = tokenize(prompt, true, true);
    if (promptTokens.empty()) {
        std::cerr << "[VieNeuTTS] Failed to tokenize prompt\n";
        return {};
    }
    std::cout << "[VieNeuTTS] prompt_tokens="
              << promptTokens.size()
              << " add_special=1"
              << " speech_start_id="
              << speechStartId
              << " speech_end_id="
              << speechEndId
              << "\n";
    if (std::find(promptTokens.begin(), promptTokens.end(), textStartId) == promptTokens.end() ||
        std::find(promptTokens.begin(), promptTokens.end(), textEndId) == promptTokens.end() ||
        std::find(promptTokens.begin(), promptTokens.end(), speechStartId) == promptTokens.end()) {
        std::cerr << "[VieNeuTTS] Prompt special tokens were not preserved by tokenizer. "
                  << "The GGUF/tokenizer pair is incompatible with this prompt format.\n";
        return {};
    }

    const int ctxSize = std::max(512, cfg_.ctx);
    if (promptTokens.size() + 1 >= static_cast<size_t>(ctxSize)) {
        std::cerr << "[VieNeuTTS] Prompt too long: " << promptTokens.size() << "\n";
        return {};
    }

    const int batchCapacity = std::max(
        1,
        std::min(std::max(1, cfg_.batch), static_cast<int>(promptTokens.size())));
    llama_sampler_chain_params samplerParams = llama_sampler_chain_default_params();
    samplerParams.no_perf = true;
    std::unique_ptr<llama_sampler, LlamaSamplerDeleter> sampler(
        llama_sampler_chain_init(samplerParams));
    if (!sampler) {
        std::cerr << "[VieNeuTTS] Failed to create llama sampler\n";
        return {};
    }
    llama_sampler_chain_add(sampler.get(), llama_sampler_init_top_k(std::max(1, cfg_.top_k)));
    llama_sampler_chain_add(sampler.get(), llama_sampler_init_top_p(std::clamp(cfg_.top_p, 0.0f, 1.0f), 1));
    llama_sampler_chain_add(sampler.get(), llama_sampler_init_min_p(std::max(0.0f, cfg_.min_p), 1));
    llama_sampler_chain_add(sampler.get(), llama_sampler_init_temp(std::max(0.0f, cfg_.temperature)));
    llama_sampler_chain_add(sampler.get(), llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

    std::string out;
    LlamaBatchGuard batchGuard(batchCapacity);
    if (!batchGuard.valid()) {
        std::cerr << "[VieNeuTTS] Failed to allocate llama batch\n";
        return {};
    }

    llama_batch& batch = batchGuard.get();

    int32_t pos = 0;

    for (size_t offset = 0; offset < promptTokens.size(); offset += static_cast<size_t>(batchCapacity)) {
        batch.n_tokens = 0;
        const size_t end = std::min(
            promptTokens.size(),
            offset + static_cast<size_t>(batchCapacity));

        for (size_t i = offset; i < end; ++i) {
            batchAdd(
                batch,
                static_cast<llama_token>(promptTokens[i]),
                pos++,
                i + 1 == promptTokens.size());
        }

        if (llama_decode(ctx_, batch) != 0) {
            std::cerr << "[VieNeuTTS] llama_decode prompt failed\n";
            return {};
        }
    }

    int generationLimit = std::max(0, max_tokens);
    if (static_cast<int>(promptTokens.size()) + generationLimit + 8 > ctxSize) {
        generationLimit = std::max(
            0,
            ctxSize - static_cast<int>(promptTokens.size()) - 8);
        std::cerr << "[VieNeuTTS] Reducing max_tokens to "
                  << generationLimit
                  << " to fit ctx="
                  << ctxSize
                  << "\n";
    }
    for (int i = 0; i < generationLimit && !stop_requested_.load(); ++i) {
        const llama_token token = llama_sampler_sample(sampler.get(), ctx_, -1);
        if (llama_vocab_is_eog(vocab_, token)) {
            break;
        }
        if (token == speechEndId) {
            break;
        }

        const std::string piece = tokenToPiece(token, true);
        if (piece == "<|SPEECH_GENERATION_END|>") {
            break;
        }

        out += piece;
        llama_sampler_accept(sampler.get(), token);

        batch.n_tokens = 0;
        batchAdd(batch, token, pos++, true);

        if (llama_decode(ctx_, batch) != 0) {
            std::cerr << "[VieNeuTTS] llama_decode token failed\n";
            break;
        }
    }
    return out;
}

std::vector<int32_t> VieNeuTtsEngine::extractSpeechIds(const std::string& text) const
{
    std::vector<int32_t> ids;
    constexpr const char* prefix = "<|speech_";
    constexpr const char* suffix = "|>";
    constexpr size_t prefixLen = 9;
    size_t pos = 0;

    while ((pos = text.find(prefix, pos)) != std::string::npos) {
        size_t valuePos = pos + prefixLen;
        int32_t value = 0;
        bool hasDigit = false;

        while (valuePos < text.size() &&
               std::isdigit(static_cast<unsigned char>(text[valuePos]))) {
            hasDigit = true;
            const int digit = text[valuePos] - '0';
            if (value > (INT32_MAX - digit) / 10) {
                hasDigit = false;
                break;
            }
            value = value * 10 + digit;
            ++valuePos;
        }

        if (hasDigit && text.compare(valuePos, 2, suffix) == 0) {
            ids.push_back(value);
            pos = valuePos + 2;
        } else {
            pos += prefixLen;
        }
    }

    return ids;
}

std::vector<float> VieNeuTtsEngine::decodeSpeechIds(const std::vector<int32_t>& codes)
{
    if (!ort_session_ || codes.empty()) {
        return {};
    }

    std::vector<int64_t> contentShape;
    if (codec_uses_voice_embedding_ || codec_content_input_rank_ == 2) {
        contentShape = {1, static_cast<int64_t>(codes.size())};
    } else {
        contentShape = {1, 1, static_cast<int64_t>(codes.size())};
    }

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
        OrtArenaAllocator,
        OrtMemTypeDefault);

    std::vector<int64_t> input64;
    Ort::Value contentTensor{nullptr};
    if (codec_content_input_element_type_ == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
        input64.reserve(codes.size());
        for (int32_t code : codes) {
            input64.push_back(static_cast<int64_t>(code));
        }
        contentTensor = Ort::Value::CreateTensor<int64_t>(
            memoryInfo,
            input64.data(),
            input64.size(),
            contentShape.data(),
            contentShape.size());
    } else {
        contentTensor = Ort::Value::CreateTensor<int32_t>(
            memoryInfo,
            const_cast<int32_t*>(codes.data()),
            codes.size(),
            contentShape.data(),
            contentShape.size());
    }

    std::vector<const char*> inputNames;
    std::vector<Ort::Value> inputTensors;
    inputNames.push_back(codec_content_input_name_.c_str());
    inputTensors.push_back(std::move(contentTensor));

    std::vector<int64_t> voiceShape;
    if (codec_uses_voice_embedding_) {
        if (voice_.embedding.empty()) {
            std::cerr << "[VieNeuTTS] Codec requires voice_embedding but no voice embedding is loaded\n";
            return {};
        }
        voiceShape = {1, static_cast<int64_t>(voice_.embedding.size())};
        inputNames.push_back(codec_voice_input_name_.c_str());
        inputTensors.push_back(Ort::Value::CreateTensor<float>(
            memoryInfo,
            const_cast<float*>(voice_.embedding.data()),
            voice_.embedding.size(),
            voiceShape.data(),
            voiceShape.size()));
    }

    const char* outputNames[] = {codec_output_name_.c_str()};

    try {
        auto outputs = ort_session_->Run(
            Ort::RunOptions{nullptr},
            inputNames.data(),
            inputTensors.data(),
            inputTensors.size(),
            outputNames,
            1);

        float* data = outputs.front().GetTensorMutableData<float>();
        const auto info = outputs.front().GetTensorTypeAndShapeInfo();
        const size_t count = info.GetElementCount();
        return std::vector<float>(data, data + count);
    } catch (const std::exception& e) {
        std::cerr << "[VieNeuTTS] ONNX codec decode failed: "
                  << e.what() << "\n";
        return {};
    }
}

std::vector<int16_t> VieNeuTtsEngine::floatToPcm16(const std::vector<float>& audio)
{
    std::vector<int16_t> pcm;
    pcm.reserve(audio.size());

    for (float sample : audio) {
        sample = std::clamp(sample, -1.0f, 1.0f);
        pcm.push_back(static_cast<int16_t>(sample * 32767.0f));
    }

    return pcm;
}

bool VieNeuTtsEngine::saveWav16(const std::string& path,
                                const std::vector<int16_t>& pcm) const
{
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        std::cerr << "[VieNeuTTS] Cannot open wav for writing: "
                  << path << "\n";
        return false;
    }

    WavHeader hdr;
    hdr.dataSize = static_cast<uint32_t>(pcm.size() * sizeof(int16_t));
    hdr.chunkSize = 36 + hdr.dataSize;
    out.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
    if (!pcm.empty()) {
        out.write(
            reinterpret_cast<const char*>(pcm.data()),
            static_cast<std::streamsize>(pcm.size() * sizeof(int16_t)));
    }

    return out.good();
}

bool VieNeuTtsEngine::savePcm16ToFile(const std::string& wav_path,
                                      const std::vector<int16_t>& pcm) const
{
    return saveWav16(wav_path, pcm);
}

bool VieNeuTtsEngine::synthesizeToPcm16(const std::string& text,
                                        std::vector<int16_t>* pcm,
                                        uint32_t* sample_rate)
{
    if (!pcm) {
        return false;
    }

    if (text.empty()) {
        std::cerr << "[VieNeuTTS] Empty text\n";
        return false;
    }

    if (!isReady() && !initialize()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    stop_requested_.store(false);
    synthesizing_.store(true);
    SynthesisStateGuard synthesisGuard{&synthesizing_};
    pcm->clear();

    const std::string prompt = buildPrompt(stripUtf8Bom(text));
    if (prompt.empty()) {
        return false;
    }

    if (cfg_.debug_prompt) {
        std::cout << "[VieNeuTTS] prompt: " << prompt << "\n";
    }

    int maxTokens = std::max(1, cfg_.max_tokens);
    const size_t charCount = utf8CodepointCount(text);
    if (cfg_.generation_tokens_per_char > 0) {
        const int estimated = std::max(1, cfg_.min_generation_tokens) +
            static_cast<int>(charCount) * cfg_.generation_tokens_per_char +
            std::max(0, cfg_.generation_tokens_extra);
        maxTokens = std::min(maxTokens, estimated);
    }
    std::cout << "[VieNeuTTS] generation max_tokens="
              << maxTokens
              << " text_chars="
              << charCount
              << "\n";

    const std::string output = generateSpeechTokens(prompt, maxTokens);
    if (stop_requested_.load()) {
        return false;
    }

    std::vector<int32_t> codes = extractSpeechIds(output);
    std::cout << "[VieNeuTTS] generated chars="
              << output.size()
              << " speech_tokens="
              << codes.size()
              << "\n";
    if (codes.empty()) {
        std::cerr << "[VieNeuTTS] No speech tokens generated\n";
        return false;
    }
    const int minSpeechTokens = std::max(1, cfg_.min_speech_tokens);
    if (codes.size() < static_cast<size_t>(minSpeechTokens)) {
        std::cerr << "[VieNeuTTS] Too few speech tokens: "
                  << codes.size()
                  << " < "
                  << minSpeechTokens
                  << ". Generated text preview: "
                  << output.substr(0, 256)
                  << "\n";
        return false;
    }

    const std::vector<float> audio = decodeSpeechIds(codes);
    if (audio.empty()) {
        return false;
    }
    const int minAudioMs = std::max(0, cfg_.min_audio_ms);
    const size_t minAudioSamples = static_cast<size_t>(
        (static_cast<uint64_t>(kSampleRate) * static_cast<uint64_t>(minAudioMs)) / 1000u);
    std::cout << "[VieNeuTTS] decoded samples="
              << audio.size()
              << " duration_ms="
              << (audio.size() * 1000u / static_cast<size_t>(kSampleRate))
              << "\n";
    if (audio.size() < minAudioSamples) {
        std::cerr << "[VieNeuTTS] Audio too short: "
                  << audio.size()
                  << " samples < "
                  << minAudioSamples
                  << " samples ("
                  << minAudioMs
                  << " ms). Not playing short burst.\n";
        return false;
    }

    *pcm = floatToPcm16(audio);
    if (sample_rate) {
        *sample_rate = kSampleRate;
    }

    return !pcm->empty();
}

bool VieNeuTtsEngine::synthesizeToFile(const std::string& text,
                                       const std::string& wav_path)
{
    std::vector<int16_t> pcm;
    uint32_t sampleRate = 0;
    if (!synthesizeToPcm16(text, &pcm, &sampleRate)) {
        return false;
    }

    (void)sampleRate;
    return saveWav16(wav_path, pcm);
}

std::string VieNeuTtsEngine::buildPrompt(const std::string& text) const
{
    const std::string promptText = preparePromptText(text);
    if (promptText.empty()) {
        return {};
    }
    std::cout << "[VieNeuTTS] prompt text preview: "
              << promptText.substr(0, 240)
              << "\n";

    const PromptMode mode = activePromptMode();
    if (mode == PromptMode::Turbo) {
        if (voice_.embedding.empty()) {
            std::cerr << "[VieNeuTTS] Voice preset has no embedding for turbo prompt\n";
            return {};
        }
        return "<|speaker_16|><|TEXT_PROMPT_START|>" +
               promptText +
               "<|TEXT_PROMPT_END|><|SPEECH_GENERATION_START|>";
    }

    if (voice_.codes.empty()) {
        std::cerr << "[VieNeuTTS] Voice preset has no prompt codes for standard prompt\n";
        return {};
    }

    const std::string refText = preparePromptText(voice_.text);
    if (refText.empty()) {
        std::cerr << "[VieNeuTTS] Voice preset has no reference text for standard prompt\n";
        return {};
    }

    // Official VieNeu GGUF prompt path mirrors pnnbao97/VieNeu-TTS standard.py.
    return "user: Convert the text to speech:<|TEXT_PROMPT_START|>" +
           refText + " " + promptText +
           "<|TEXT_PROMPT_END|>\nassistant:<|SPEECH_GENERATION_START|>" +
           speechCodesToText(voice_.codes);
}

std::string VieNeuTtsEngine::preparePromptText(const std::string& text) const
{
    std::string out = normalizePromptText(text);
    if (!phoneme_dict_.empty()) {
        out = phonemizeWithDictionary(out);
    }
    return trim(out);
}

std::string VieNeuTtsEngine::normalizePromptText(const std::string& text) const
{
    std::string clean = stripUtf8Bom(text);
    if (clean.empty()) {
        return {};
    }

    std::string out;
    out.reserve(clean.size());
    bool pendingSpace = false;
    for (char c : clean) {
        const unsigned char uc = static_cast<unsigned char>(c);
        if (std::isspace(uc)) {
            pendingSpace = true;
            continue;
        }

        if (c == '\n' || c == '\r' || c == '\t') {
            pendingSpace = true;
            continue;
        }

        if (pendingSpace && !out.empty()) {
            out.push_back(' ');
        }
        pendingSpace = false;
        out.push_back(c);
    }

    out = normalizeNumbers(trim(out));
    return trim(out);
}

std::string VieNeuTtsEngine::phonemizeWithDictionary(const std::string& text) const
{
    std::string out;
    std::string token;
    std::vector<std::string> unknownPreview;
    int unknownCount = 0;
    bool hasUnknown = false;
    out.reserve(text.size() * 2);
    token.reserve(64);

    auto appendSpace = [&out]() {
        if (!out.empty() && out.back() != ' ') {
            out.push_back(' ');
        }
    };

    auto appendPunctuation = [&out](char c) {
        while (!out.empty() && out.back() == ' ') {
            out.pop_back();
        }
        out.push_back(c);
        out.push_back(' ');
    };

    auto lookupPhoneme = [&](const std::string& key, std::string* phoneme) {
        if (!phoneme || key.empty()) {
            return false;
        }
        const auto it = phoneme_dict_.find(key);
        if (it == phoneme_dict_.end()) {
            return false;
        }
        *phoneme = it->second;
        return true;
    };

    auto appendPart = [](std::string* dst, const std::string& part) {
        if (!dst || part.empty()) {
            return;
        }
        if (!dst->empty() && dst->back() != ' ') {
            dst->push_back(' ');
        }
        *dst += part;
    };

    auto appendDigitPhoneme = [&](std::string* dst, char digit) {
        std::string phoneme;
        if (!lookupPhoneme(vietnameseDigitWord(digit), &phoneme)) {
            return false;
        }
        appendPart(dst, phoneme);
        return true;
    };

    auto appendEnglishSpelling = [&](std::string* dst, const std::string& token) {
        if (!dst) {
            return false;
        }
        std::string spelled;
        for (char raw : token) {
            if (isAsciiAlpha(raw)) {
                appendPart(&spelled, englishLetterPhoneme(raw));
            } else if (std::isdigit(static_cast<unsigned char>(raw))) {
                if (!appendDigitPhoneme(&spelled, raw)) {
                    return false;
                }
            } else {
                return false;
            }
        }
        appendPart(dst, spelled);
        return !spelled.empty();
    };

    auto appendMappedWords = [&](std::string* dst,
                                 const std::initializer_list<const char*> words) {
        for (const char* word : words) {
            std::string phoneme;
            if (!lookupPhoneme(word, &phoneme)) {
                return false;
            }
            appendPart(dst, phoneme);
        }
        return true;
    };

    auto tokenIsAsciiAlnum = [](const std::string& value) {
        return !value.empty() &&
               std::all_of(value.begin(), value.end(), isAsciiAlnum);
    };

    auto tokenAllAsciiUpper = [](const std::string& value) {
        bool hasAlpha = false;
        for (char c : value) {
            if (!isAsciiAlpha(c)) {
                continue;
            }
            hasAlpha = true;
            if (!isAsciiUpperAlpha(c)) {
                return false;
            }
        }
        return hasAlpha;
    };

    auto appendTechnicalToken = [&](const std::string& raw,
                                    const std::string& lowered,
                                    std::string* phoneme) {
        if (!phoneme || !tokenIsAsciiAlnum(raw)) {
            return false;
        }

        static const std::unordered_map<std::string, const char*> kDirect = {
            {"ai", "ˈeɪ ˈaɪ"},
            {"api", "ˈeɪ pˈiː ˈaɪ"},
            {"asr", "ˈeɪ ˈɛs ˈɑːɹ"},
            {"chatgpt", "tʃˈæt dʒˌiːpˌiːtˈiː"},
            {"cpu", "sˈiː pˈiː jˈuː"},
            {"gpt", "dʒˌiːpˌiːtˈiː"},
            {"groq", "dʒˈiː ˈɑːɹ ˈoʊ kjˈuː"},
            {"gpu", "dʒˈiː pˈiː jˈuː"},
            {"ggml", "dʒˈiː dʒˈiː ˈɛm ˈɛl"},
            {"gguf", "dʒˈiː dʒˈiː jˈuː ˈɛf"},
            {"llama", "lˈɑːmə"},
            {"llm", "ˌɛl ˌɛl ˈɛm"},
            {"onnx", "ˈoʊ ˈɛn ˈɛn ˈɛks"},
            {"openai", "ˈoʊpən ˈeɪ ˈaɪ"},
            {"piper", "pˈaɪpɚ"},
            {"qml", "kjˈuː ˈɛm ˈɛl"},
            {"qt", "kjˈuː tˈiː"},
            {"tts", "tˈiː tˈiː ˈɛs"},
            {"url", "jˈuː ˈɑːɹ ˈɛl"},
            {"usb", "jˈuː ˈɛs bˈiː"},
            {"vieneu", "vˈaɪ nˈuː"},
            {"wifi", "wˈaɪ fˈaɪ"}
        };

        const bool hasDigit = std::any_of(
            raw.begin(),
            raw.end(),
            [](char c) { return std::isdigit(static_cast<unsigned char>(c)); });
        const bool hasInternalUpper = raw.size() > 1 &&
            std::any_of(raw.begin() + 1, raw.end(), isAsciiUpperAlpha);
        const auto direct = kDirect.find(lowered);
        const bool hasDirect = direct != kDirect.end();
        const bool shouldTreatAsTechnical =
            hasDirect || tokenAllAsciiUpper(raw) || hasDigit || hasInternalUpper;
        if (!shouldTreatAsTechnical) {
            return false;
        }
        if (lowered == "ai" && !tokenAllAsciiUpper(raw)) {
            return false;
        }
        if (!hasDirect && hasInternalUpper && phoneme_dict_.find(lowered) != phoneme_dict_.end()) {
            return false;
        }
        if (hasDirect) {
            appendPart(phoneme, direct->second);
            return true;
        }

        if (!lowered.empty() && lowered[0] == 'q' && lowered.size() > 1) {
            std::string qPhoneme;
            if (!lookupPhoneme("qui", &qPhoneme)) {
                qPhoneme = "kwj";
            }
            appendPart(phoneme, qPhoneme);
            for (size_t i = 1; i < lowered.size(); ++i) {
                const char c = lowered[i];
                if (std::isdigit(static_cast<unsigned char>(c))) {
                    if (!appendDigitPhoneme(phoneme, c)) {
                        return false;
                    }
                } else if (isAsciiAlpha(c)) {
                    appendPart(phoneme, englishLetterPhoneme(c));
                } else {
                    return false;
                }
            }
            return true;
        }

        if (lowered == "pi") {
            return appendMappedWords(phoneme, {"pi"});
        }

        return appendEnglishSpelling(phoneme, raw);
    };

    auto flushToken = [&]() {
        if (token.empty()) {
            return;
        }

        const std::string rawToken = token;
        const auto exact = phoneme_dict_.find(token);
        if (exact != phoneme_dict_.end()) {
            appendSpace();
            out += exact->second;
            token.clear();
            return;
        }

        const std::string lowered = lowerVietnameseUtf8(token);
        const auto lower = phoneme_dict_.find(lowered);
        appendSpace();
        std::string technicalPhoneme;
        if (appendTechnicalToken(rawToken, lowered, &technicalPhoneme)) {
            out += technicalPhoneme;
        } else if (lower != phoneme_dict_.end()) {
            out += lower->second;
        } else if (tokenIsAsciiAlnum(rawToken) &&
                   tokenAllAsciiUpper(rawToken) &&
                   appendEnglishSpelling(&technicalPhoneme, rawToken)) {
            out += technicalPhoneme;
            token.clear();
            return;
        } else {
            ++unknownCount;
            hasUnknown = true;
            if (unknownPreview.size() <
                static_cast<size_t>(std::max(0, cfg_.max_unknown_phoneme_preview))) {
                unknownPreview.push_back(lowered);
            }
            if (cfg_.allow_raw_phoneme_fallback) {
                out += lowered;
            }
        }
        token.clear();
    };

    for (size_t i = 0; i < text.size();) {
        const size_t before = i;
        uint32_t cp = 0;
        const bool valid = decodeUtf8Codepoint(text, &i, &cp);
        if (!valid) {
            flushToken();
            appendSpace();
            continue;
        }

        if (cp < 0x80) {
            const unsigned char c = static_cast<unsigned char>(cp);
            if (std::isalnum(c)) {
                token.push_back(static_cast<char>(c));
                continue;
            }
            flushToken();
            if (std::isspace(c)) {
                appendSpace();
                continue;
            }
            if (c == '.' || c == ',' || c == '!' || c == '?' ||
                c == ';' || c == ':') {
                appendPunctuation(static_cast<char>(c));
                continue;
            }
            appendSpace();
            continue;
        }

        if (cp == 0x2026) { // ellipsis
            flushToken();
            appendPunctuation('.');
            continue;
        }
        if ((cp >= 0x2010 && cp <= 0x2015) ||
            cp == 0x2212 ||
            cp == 0x2018 ||
            cp == 0x2019 ||
            cp == 0x201C ||
            cp == 0x201D ||
            cp == 0x00AB ||
            cp == 0x00BB) {
            flushToken();
            appendSpace();
            continue;
        }

        token.append(text, before, i - before);
    }

    flushToken();
    if (hasUnknown && !cfg_.allow_raw_phoneme_fallback) {
        std::cerr << "[VieNeuTTS] phoneme dictionary missing "
                  << unknownCount
                  << " token(s); refusing raw-text fallback because it makes VieNeu speak unnaturally.";
        if (!unknownPreview.empty()) {
            std::cerr << " preview=";
            for (size_t i = 0; i < unknownPreview.size(); ++i) {
                if (i > 0) {
                    std::cerr << ",";
                }
                std::cerr << unknownPreview[i];
            }
        }
        std::cerr << "\n";
        return {};
    }
    if (hasUnknown) {
        std::cerr << "[VieNeuTTS] phoneme dictionary missing "
                  << unknownCount
                  << " token(s); using raw fallback for preview only.\n";
    }
    return trim(out);
}

VieNeuTtsEngine::PromptMode VieNeuTtsEngine::configuredPromptMode() const
{
    if (equalsIgnoreCase(cfg_.prompt_mode, "standard") ||
        equalsIgnoreCase(cfg_.prompt_mode, "legacy")) {
        return PromptMode::Standard;
    }
    if (equalsIgnoreCase(cfg_.prompt_mode, "turbo") ||
        equalsIgnoreCase(cfg_.prompt_mode, "v2")) {
        return PromptMode::Turbo;
    }
    return PromptMode::Auto;
}

VieNeuTtsEngine::PromptMode VieNeuTtsEngine::activePromptMode() const
{
    const PromptMode requested = configuredPromptMode();
    if (requested != PromptMode::Auto) {
        return requested;
    }
    return codec_uses_voice_embedding_ ? PromptMode::Turbo : PromptMode::Standard;
}

std::string VieNeuTtsEngine::speechCodesToText(const std::vector<int32_t>& codes) const
{
    std::string out;
    out.reserve(codes.size() * 16);
    for (int32_t code : codes) {
        out += "<|speech_";
        out += std::to_string(code);
        out += "|>";
    }
    return out;
}

std::string VieNeuTtsEngine::normalizeNumbers(std::string text)
{
    static constexpr const char* kDigits[] = {
        "không",
        "một",
        "hai",
        "ba",
        "bốn",
        "năm",
        "sáu",
        "bảy",
        "tám",
        "chín"
    };

    auto numberToWords = [](int value) -> std::string {
        if (value < 0 || value > 99) {
            return std::to_string(value);
        }
        if (value < 10) {
            return kDigits[value];
        }
        const int tens = value / 10;
        const int ones = value % 10;
        std::string out;
        if (tens == 1) {
            out = "mười";
        } else {
            out = std::string(kDigits[tens]) + " mươi";
        }
        if (ones == 0) {
            return out;
        }
        if (ones == 1 && tens > 1) {
            return out + " mốt";
        }
        if (ones == 5) {
            return out + " lăm";
        }
        return out + " " + kDigits[ones];
    };

    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size();) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) {
            out.push_back(text[i++]);
            continue;
        }

        size_t j = i;
        int value = 0;
        bool smallEnough = true;
        while (j < text.size() &&
               std::isdigit(static_cast<unsigned char>(text[j]))) {
            const int digit = text[j] - '0';
            if (value > 9999) {
                smallEnough = false;
            }
            value = value * 10 + digit;
            ++j;
        }

        if ((i > 0 && isAsciiAlpha(text[i - 1])) ||
            (j < text.size() && isAsciiAlpha(text[j]))) {
            out.append(text, i, j - i);
            i = j;
            continue;
        }

        std::string replacement;
        if (smallEnough && value <= 99) {
            replacement = numberToWords(value);
        } else {
            replacement.assign(text, i, j - i);
        }
        if (i > 0 &&
            std::isalnum(static_cast<unsigned char>(text[i - 1])) &&
            !out.empty() &&
            out.back() != ' ') {
            out.push_back(' ');
        }
        out += replacement;
        if (j < text.size() &&
            std::isalnum(static_cast<unsigned char>(text[j])) &&
            !out.empty() &&
            out.back() != ' ') {
            out.push_back(' ');
        }
        i = j;
    }

    return out;
}

std::string VieNeuTtsEngine::stripUtf8Bom(std::string text)
{
    if (text.size() >= 3 &&
        static_cast<unsigned char>(text[0]) == 0xEF &&
        static_cast<unsigned char>(text[1]) == 0xBB &&
        static_cast<unsigned char>(text[2]) == 0xBF) {
        text.erase(0, 3);
    }
    return text;
}

std::string VieNeuTtsEngine::readFile(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return {};
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}
