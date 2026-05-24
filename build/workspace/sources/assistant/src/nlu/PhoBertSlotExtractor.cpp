#include "PhoBertSlotExtractor.h"

#include <QMap>
#include <QPair>
#include <QRegularExpression>
#include <QStringList>
#include <QVector>

#include <algorithm>

namespace nlu {
namespace {

QString stripVietnameseDiacritics(QString text)
{
    static const QVector<QPair<QString, QString>> replacements = {
        {QStringLiteral("à"), QStringLiteral("a")}, {QStringLiteral("á"), QStringLiteral("a")},
        {QStringLiteral("ả"), QStringLiteral("a")}, {QStringLiteral("ã"), QStringLiteral("a")},
        {QStringLiteral("ạ"), QStringLiteral("a")}, {QStringLiteral("ă"), QStringLiteral("a")},
        {QStringLiteral("ằ"), QStringLiteral("a")}, {QStringLiteral("ắ"), QStringLiteral("a")},
        {QStringLiteral("ẳ"), QStringLiteral("a")}, {QStringLiteral("ẵ"), QStringLiteral("a")},
        {QStringLiteral("ặ"), QStringLiteral("a")}, {QStringLiteral("â"), QStringLiteral("a")},
        {QStringLiteral("ầ"), QStringLiteral("a")}, {QStringLiteral("ấ"), QStringLiteral("a")},
        {QStringLiteral("ẩ"), QStringLiteral("a")}, {QStringLiteral("ẫ"), QStringLiteral("a")},
        {QStringLiteral("ậ"), QStringLiteral("a")}, {QStringLiteral("è"), QStringLiteral("e")},
        {QStringLiteral("é"), QStringLiteral("e")}, {QStringLiteral("ẻ"), QStringLiteral("e")},
        {QStringLiteral("ẽ"), QStringLiteral("e")}, {QStringLiteral("ẹ"), QStringLiteral("e")},
        {QStringLiteral("ê"), QStringLiteral("e")}, {QStringLiteral("ề"), QStringLiteral("e")},
        {QStringLiteral("ế"), QStringLiteral("e")}, {QStringLiteral("ể"), QStringLiteral("e")},
        {QStringLiteral("ễ"), QStringLiteral("e")}, {QStringLiteral("ệ"), QStringLiteral("e")},
        {QStringLiteral("ì"), QStringLiteral("i")}, {QStringLiteral("í"), QStringLiteral("i")},
        {QStringLiteral("ỉ"), QStringLiteral("i")}, {QStringLiteral("ĩ"), QStringLiteral("i")},
        {QStringLiteral("ị"), QStringLiteral("i")}, {QStringLiteral("ò"), QStringLiteral("o")},
        {QStringLiteral("ó"), QStringLiteral("o")}, {QStringLiteral("ỏ"), QStringLiteral("o")},
        {QStringLiteral("õ"), QStringLiteral("o")}, {QStringLiteral("ọ"), QStringLiteral("o")},
        {QStringLiteral("ô"), QStringLiteral("o")}, {QStringLiteral("ồ"), QStringLiteral("o")},
        {QStringLiteral("ố"), QStringLiteral("o")}, {QStringLiteral("ổ"), QStringLiteral("o")},
        {QStringLiteral("ỗ"), QStringLiteral("o")}, {QStringLiteral("ộ"), QStringLiteral("o")},
        {QStringLiteral("ơ"), QStringLiteral("o")}, {QStringLiteral("ờ"), QStringLiteral("o")},
        {QStringLiteral("ớ"), QStringLiteral("o")}, {QStringLiteral("ở"), QStringLiteral("o")},
        {QStringLiteral("ỡ"), QStringLiteral("o")}, {QStringLiteral("ợ"), QStringLiteral("o")},
        {QStringLiteral("ù"), QStringLiteral("u")}, {QStringLiteral("ú"), QStringLiteral("u")},
        {QStringLiteral("ủ"), QStringLiteral("u")}, {QStringLiteral("ũ"), QStringLiteral("u")},
        {QStringLiteral("ụ"), QStringLiteral("u")}, {QStringLiteral("ư"), QStringLiteral("u")},
        {QStringLiteral("ừ"), QStringLiteral("u")}, {QStringLiteral("ứ"), QStringLiteral("u")},
        {QStringLiteral("ử"), QStringLiteral("u")}, {QStringLiteral("ữ"), QStringLiteral("u")},
        {QStringLiteral("ự"), QStringLiteral("u")}, {QStringLiteral("ỳ"), QStringLiteral("y")},
        {QStringLiteral("ý"), QStringLiteral("y")}, {QStringLiteral("ỷ"), QStringLiteral("y")},
        {QStringLiteral("ỹ"), QStringLiteral("y")}, {QStringLiteral("ỵ"), QStringLiteral("y")},
        {QStringLiteral("đ"), QStringLiteral("d")}
    };

    for (const auto& item : replacements) {
        text.replace(item.first, item.second, Qt::CaseInsensitive);
    }
    return text;
}

QString cleanText(QString text)
{
    text = text.toLower().trimmed();
    text.replace(QRegularExpression(QStringLiteral("[\\.,;:!?\\[\\]\\(\\)\\{\\}\\\"']+")),
                 QStringLiteral(" "));
    text.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    return text.trimmed();
}

QString normalizedForMatch(QString text)
{
    return stripVietnameseDiacritics(cleanText(text));
}

int digitWordValue(const QString& word)
{
    static const QMap<QString, int> values = {
        {QStringLiteral("khong"), 0}, {QStringLiteral("linh"), 0}, {QStringLiteral("le"), 0},
        {QStringLiteral("mot"), 1}, {QStringLiteral("mots"), 1}, {QStringLiteral("moi"), 1},
        {QStringLiteral("hai"), 2}, {QStringLiteral("ba"), 3}, {QStringLiteral("bon"), 4},
        {QStringLiteral("tu"), 4}, {QStringLiteral("nam"), 5}, {QStringLiteral("lam"), 5},
        {QStringLiteral("nham"), 5}, {QStringLiteral("sau"), 6}, {QStringLiteral("bay"), 7},
        {QStringLiteral("tam"), 8}, {QStringLiteral("chin"), 9}
    };
    return values.value(word, -1);
}

bool isNumberControlWord(const QString& word)
{
    return word == QStringLiteral("muoi") ||
           word == QStringLiteral("chuc") ||
           word == QStringLiteral("tram") ||
           word == QStringLiteral("linh") ||
           word == QStringLiteral("le");
}

QStringList numberWords(QString phrase)
{
    phrase = normalizedForMatch(phrase);
    QStringList out;
    const QStringList words = phrase.split(QRegularExpression(QStringLiteral("\\s+")),
                                           Qt::SkipEmptyParts);
    for (const QString& word : words) {
        if (digitWordValue(word) >= 0 || isNumberControlWord(word)) {
            out.push_back(word);
        }
    }
    return out;
}

int parseUnder100(const QStringList& words)
{
    if (words.isEmpty()) return -1;

    int tensIndex = -1;
    for (int i = 0; i < words.size(); ++i) {
        if (words[i] == QStringLiteral("muoi") || words[i] == QStringLiteral("chuc")) {
            tensIndex = i;
            break;
        }
    }

    if (tensIndex >= 0) {
        int tens = 10;
        if (tensIndex > 0) {
            const int prefix = digitWordValue(words[tensIndex - 1]);
            if (prefix > 0) tens = prefix * 10;
        }

        int ones = 0;
        for (int i = tensIndex + 1; i < words.size(); ++i) {
            const int digit = digitWordValue(words[i]);
            if (digit >= 0) {
                ones = digit;
                break;
            }
        }
        return tens + ones;
    }

    QVector<int> digits;
    for (const QString& word : words) {
        const int digit = digitWordValue(word);
        if (digit >= 0) digits.push_back(digit);
    }

    if (digits.isEmpty()) return -1;
    if (digits.size() == 1) return digits.first();
    if (digits.size() == 2 && digits.first() > 0) return digits[0] * 10 + digits[1];

    int value = 0;
    for (int digit : digits) value = value * 10 + digit;
    return value;
}

int parseVietnameseNumberPhrase(QString phrase)
{
    phrase = normalizedForMatch(phrase);

    bool ok = false;
    const int direct = phrase.toInt(&ok);
    if (ok) return direct;

    const QStringList words = numberWords(phrase);
    if (words.isEmpty()) return -1;

    for (int i = 0; i < words.size(); ++i) {
        if (words[i] != QStringLiteral("tram")) continue;

        int hundreds = 1;
        if (i > 0) {
            const int prefix = digitWordValue(words[i - 1]);
            if (prefix > 0) hundreds = prefix;
        }

        const int tail = parseUnder100(words.mid(i + 1));
        return hundreds * 100 + std::max(0, tail);
    }

    return parseUnder100(words);
}

int firstNumberInRange(const QString& text, int minValue, int maxValue)
{
    const QString normalized = normalizedForMatch(text);

    const QRegularExpression digitRe(QStringLiteral("(^|\\D)(\\d{1,3})(\\D|$)"));
    auto it = digitRe.globalMatch(normalized);
    while (it.hasNext()) {
        const auto match = it.next();
        bool ok = false;
        const int value = match.captured(2).toInt(&ok);
        if (ok && value >= minValue && value <= maxValue) return value;
    }

    const QRegularExpression wordsRe(QStringLiteral(
        "\\b(khong|linh|le|mot|mots|moi|hai|ba|bon|tu|nam|lam|nham|sau|bay|tam|chin|muoi|chuc|tram)(?:\\s+(khong|linh|le|mot|mots|moi|hai|ba|bon|tu|nam|lam|nham|sau|bay|tam|chin|muoi|chuc|tram))*\\b"));
    auto wit = wordsRe.globalMatch(normalized);
    while (wit.hasNext()) {
        const int value = parseVietnameseNumberPhrase(wit.next().captured(0));
        if (value >= minValue && value <= maxValue) return value;
    }

    return -1;
}

int levelFromText(const QString& text, int minValue, int maxValue)
{
    const QString normalized = normalizedForMatch(text);

    if (normalized.contains(QStringLiteral("cao nhat")) ||
        normalized.contains(QStringLiteral("lon nhat")) ||
        normalized.contains(QStringLiteral("toi da")) ||
        normalized.contains(QStringLiteral("max"))) {
        return maxValue;
    }

    if (normalized.contains(QStringLiteral("thap nhat")) ||
        normalized.contains(QStringLiteral("nho nhat")) ||
        normalized.contains(QStringLiteral("toi thieu")) ||
        normalized.contains(QStringLiteral("min"))) {
        return minValue;
    }

    return firstNumberInRange(text, minValue, maxValue);
}

bool isAsciiDigits(const QString& token)
{
    if (token.isEmpty()) return false;
    for (const QChar& ch : token) {
        if (!ch.isDigit()) return false;
    }
    return true;
}

QString phoneDigitsFromText(const QString& text)
{
    const QString normalized = normalizedForMatch(text);
    QString digits;

    const QRegularExpression tokenRe(QStringLiteral("\\d+|[a-z]+"));
    auto it = tokenRe.globalMatch(normalized);
    while (it.hasNext()) {
        const QString token = it.next().captured(0);
        if (isAsciiDigits(token)) {
            digits += token;
            continue;
        }

        const int digit = digitWordValue(token);
        if (digit >= 0) {
            digits += QString::number(digit);
        } else if (token == QStringLiteral("muoi")) {
            digits += QStringLiteral("10");
        }
    }

    return digits;
}

bool isValidPhoneNumber(const QString& digits)
{
    return digits.size() == 10 && digits.startsWith(QStringLiteral("0"));
}

QString trimSlotText(QString value)
{
    value = cleanText(value);

    static const QStringList leadingFillers = {
        QStringLiteral("cho toi"), QStringLiteral("cho tôi"),
        QStringLiteral("toi"), QStringLiteral("tôi"),
        QStringLiteral("den"), QStringLiteral("đến"),
        QStringLiteral("toi den"), QStringLiteral("tới"),
        QStringLiteral("ve"), QStringLiteral("về"),
        QStringLiteral("bai hat"), QStringLiteral("bài hát"),
        QStringLiteral("bai"), QStringLiteral("bài"),
        QStringLiteral("nhac"), QStringLiteral("nhạc"),
        QStringLiteral("nghe si"), QStringLiteral("nghệ sĩ"),
        QStringLiteral("ca si"), QStringLiteral("ca sĩ"),
        QStringLiteral("so dien thoai"), QStringLiteral("số điện thoại"),
        QStringLiteral("so"), QStringLiteral("số")
    };

    bool changed = true;
    while (changed) {
        changed = false;
        for (const QString& filler : leadingFillers) {
            if (value.startsWith(filler + QStringLiteral(" "))) {
                value = value.mid(filler.size()).trimmed();
                changed = true;
            }
        }
    }

    static const QStringList stopPhrases = {
        QStringLiteral("tren youtube"), QStringLiteral("trên youtube"),
        QStringLiteral("youtube"),
        QStringLiteral("tren spotify"), QStringLiteral("trên spotify"),
        QStringLiteral("spotify"),
        QStringLiteral("tren zing"), QStringLiteral("trên zing"),
        QStringLiteral("zing mp3"),
        QStringLiteral("bang o to"), QStringLiteral("bằng ô tô"),
        QStringLiteral("bang xe hoi"), QStringLiteral("bằng xe hơi"),
        QStringLiteral("bang xe"), QStringLiteral("bằng xe"),
        QStringLiteral("tu vi tri hien tai"), QStringLiteral("từ vị trí hiện tại"),
        QStringLiteral("tu vi tri cua toi"), QStringLiteral("từ vị trí của tôi"),
        QStringLiteral("tu day"), QStringLiteral("từ đây"),
        QStringLiteral("roi phat"), QStringLiteral("rồi phát"),
        QStringLiteral("roi mo"), QStringLiteral("rồi mở"),
        QStringLiteral("roi dan duong"), QStringLiteral("rồi dẫn đường"),
        QStringLiteral("ngay bay gio"), QStringLiteral("ngay bây giờ"),
        QStringLiteral("cho toi"), QStringLiteral("cho tôi"),
        QStringLiteral("giup toi"), QStringLiteral("giúp tôi"),
        QStringLiteral("tro ly"), QStringLiteral("trợ lý")
    };

    const QString normalized = normalizedForMatch(value);
    int cutIndex = value.size();
    for (const QString& stopPhrase : stopPhrases) {
        const QString stop = normalizedForMatch(stopPhrase);
        const QRegularExpression re(
            QStringLiteral("(^|\\s)%1(\\s|$)").arg(QRegularExpression::escape(stop)),
            QRegularExpression::CaseInsensitiveOption
        );
        const auto match = re.match(normalized);
        if (match.hasMatch()) {
            cutIndex = std::min(cutIndex, static_cast<int>(match.capturedStart(0)));
        }
    }
    if (cutIndex < value.size()) {
        value = value.left(cutIndex).trimmed();
    }

    static const QStringList trailingFillers = {
        QStringLiteral("giup toi"), QStringLiteral("giúp tôi"),
        QStringLiteral("giup"), QStringLiteral("giúp"),
        QStringLiteral("nhe"), QStringLiteral("nhé"),
        QStringLiteral("nha"), QStringLiteral("voi"),
        QStringLiteral("với"), QStringLiteral("di"), QStringLiteral("đi"),
        QStringLiteral("a"), QStringLiteral("ạ"),
        QStringLiteral("luon"), QStringLiteral("luôn"),
        QStringLiteral("ngay"),
        QStringLiteral("nua"), QStringLiteral("nữa"),
        QStringLiteral("gan day"), QStringLiteral("gần đây"),
        QStringLiteral("quanh day"), QStringLiteral("quanh đây"),
        QStringLiteral("xung quanh")
    };

    changed = true;
    while (changed) {
        changed = false;
        for (const QString& filler : trailingFillers) {
            if (value.endsWith(QStringLiteral(" ") + filler)) {
                value.chop(filler.size());
                value = value.trimmed();
                changed = true;
            }
        }
    }

    return value;
}

QString afterLastKeyword(const QString& text, const QStringList& keywords)
{
    const QString searchable = normalizedForMatch(text);
    const QString cleaned = cleanText(text);
    QString best;
    int bestEnd = -1;

    for (const QString& rawKeyword : keywords) {
        const QString keyword = normalizedForMatch(rawKeyword);
        const QRegularExpression re(
            QStringLiteral("(^|\\s)%1(\\s|$)").arg(QRegularExpression::escape(keyword)),
            QRegularExpression::CaseInsensitiveOption
        );
        auto it = re.globalMatch(searchable);
        while (it.hasNext()) {
            const auto match = it.next();
            const int end = static_cast<int>(match.capturedEnd(1) + keyword.size());
            if (end >= cleaned.size()) {
                continue;
            }

            const QString candidate = trimSlotText(cleaned.mid(end));
            if (candidate.isEmpty()) {
                continue;
            }

            if (end > bestEnd) {
                best = candidate;
                bestEnd = end;
            }
        }
    }

    return best;
}

QString trimPhoneNumberIntro(QString value)
{
    value = trimSlotText(value);

    const QString normalized = normalizedForMatch(value);
    static const QStringList introPrefixes = {
        QStringLiteral("sau day"),
        QStringLiteral("sau la"),
        QStringLiteral("sau cho"),
        QStringLiteral("sau nhe"),
        QStringLiteral("sau nha")
    };

    for (const QString& prefix : introPrefixes) {
        if (normalized == prefix) {
            return {};
        }
        if (normalized.startsWith(prefix + QStringLiteral(" "))) {
            return trimSlotText(value.mid(prefix.size()));
        }
    }

    return value;
}

QString phoneNumberSlotText(const QString& text)
{
    const QString candidate = afterLastKeyword(text, {
        QStringLiteral("số điện thoại"),
        QStringLiteral("số máy"),
        QStringLiteral("gọi vào số"),
        QStringLiteral("gọi tới số"),
        QStringLiteral("gọi đến số"),
        QStringLiteral("gọi số"),
        QStringLiteral("quay số"),
        QStringLiteral("bấm số"),
        QStringLiteral("số"),
        QStringLiteral("gọi vào"),
        QStringLiteral("gọi tới"),
        QStringLiteral("gọi đến"),
        QStringLiteral("gọi")
    });

    return candidate.isEmpty() ? text : trimPhoneNumberIntro(candidate);
}

QString knownSlotValueFromText(const QString& text, const QStringList& values)
{
    const QString cleaned = cleanText(text);
    const QString normalized = normalizedForMatch(text);
    QString best;
    int bestSize = -1;

    for (const QString& value : values) {
        const QString rawCandidate = cleanText(value);
        const QRegularExpression rawRe(
            QStringLiteral("(^|\\s)%1(\\s|$)").arg(QRegularExpression::escape(rawCandidate)),
            QRegularExpression::CaseInsensitiveOption
        );
        if (!rawCandidate.isEmpty() &&
            cleaned.contains(rawRe) &&
            rawCandidate.size() > bestSize) {
            best = value;
            bestSize = static_cast<int>(rawCandidate.size());
            continue;
        }

        const QString candidate = normalizedForMatch(value);
        if (candidate.size() < 2 || candidate == QStringLiteral("nha")) {
            continue;
        }
        const QRegularExpression normalizedRe(
            QStringLiteral("(^|\\s)%1(\\s|$)").arg(QRegularExpression::escape(candidate)),
            QRegularExpression::CaseInsensitiveOption
        );
        if (!candidate.isEmpty() &&
            normalized.contains(normalizedRe) &&
            candidate.size() > bestSize) {
            best = value;
            bestSize = static_cast<int>(candidate.size());
        }
    }

    return best;
}

const QStringList& artistValues()
{
    static const QStringList values = {
        QStringLiteral("Amee"),
        QStringLiteral("Bích Phương"),
        QStringLiteral("HIEUTHUHAI"),
        QStringLiteral("Hoàng Thùy Linh"),
        QStringLiteral("Jack"),
        QStringLiteral("JustaTee"),
        QStringLiteral("Mono"),
        QStringLiteral("Mỹ Tâm"),
        QStringLiteral("Noo Phước Thịnh"),
        QStringLiteral("Phương Mỹ Chi"),
        QStringLiteral("Sơn Tùng"),
        QStringLiteral("Trúc Nhân"),
        QStringLiteral("Vũ"),
        QStringLiteral("Đen Vâu")
    };
    return values;
}

const QStringList& songValues()
{
    static const QStringList values = {
        QStringLiteral("Bên trên tầng lầu"),
        QStringLiteral("Chúng ta của hiện tại"),
        QStringLiteral("Có chắc yêu là đây"),
        QStringLiteral("Cắt đôi nỗi sầu"),
        QStringLiteral("Em của ngày hôm qua"),
        QStringLiteral("Hẹn em kiếp sau"),
        QStringLiteral("Lạc Trôi"),
        QStringLiteral("Một vòng Việt Nam"),
        QStringLiteral("Ngày mai người ta lấy chồng"),
        QStringLiteral("Nơi này có anh"),
        QStringLiteral("See Tình"),
        QStringLiteral("Waiting For You"),
        QStringLiteral("Đi về nhà"),
        QStringLiteral("Đưa nhau đi trốn")
    };
    return values;
}

const QStringList& contactValues()
{
    static const QStringList values = {
        QStringLiteral("Minh Anh"),
        QStringLiteral("Quốc Bảo"),
        QStringLiteral("anh Nam"),
        QStringLiteral("anh tài xế"),
        QStringLiteral("ba"),
        QStringLiteral("bà nội"),
        QStringLiteral("bác sĩ Minh"),
        QStringLiteral("bạn Long"),
        QStringLiteral("bố"),
        QStringLiteral("chị Lan"),
        QStringLiteral("chị kế toán"),
        QStringLiteral("chồng"),
        QStringLiteral("cô Hạnh"),
        QStringLiteral("em Trang"),
        QStringLiteral("má"),
        QStringLiteral("mẹ"),
        QStringLiteral("sếp Hùng"),
        QStringLiteral("thằng Tuấn"),
        QStringLiteral("vợ"),
        QStringLiteral("ông ngoại")
    };
    return values;
}

const QStringList& destinationValues()
{
    static const QStringList values = {
        QStringLiteral("Hồ Gươm"),
        QStringLiteral("bến xe miền Đông"),
        QStringLiteral("bệnh viện Chợ Rẫy"),
        QStringLiteral("chợ Bến Thành"),
        QStringLiteral("cây xăng gần đây"),
        QStringLiteral("công ty"),
        QStringLiteral("garage ô tô"),
        QStringLiteral("nhà"),
        QStringLiteral("nhà mẹ"),
        QStringLiteral("phòng khám gần đây"),
        QStringLiteral("quán cà phê gần đây"),
        QStringLiteral("siêu thị gần nhất"),
        QStringLiteral("sân bay Tân Sơn Nhất"),
        QStringLiteral("trường học của con"),
        QStringLiteral("Đà Nẵng center")
    };
    return values;
}

const QStringList& poiValues()
{
    static const QStringList values = {
        QStringLiteral("ATM"),
        QStringLiteral("bãi đỗ xe"),
        QStringLiteral("bệnh viện"),
        QStringLiteral("cây xăng"),
        QStringLiteral("garage"),
        QStringLiteral("khách sạn"),
        QStringLiteral("nhà hàng"),
        QStringLiteral("nhà thuốc"),
        QStringLiteral("quán cà phê"),
        QStringLiteral("rửa xe"),
        QStringLiteral("siêu thị"),
        QStringLiteral("trạm dừng chân"),
        QStringLiteral("trạm sạc")
    };
    return values;
}

const QStringList& vehicleMetricValues()
{
    static const QStringList values = {
        QStringLiteral("cốp xe"),
        QStringLiteral("cửa xe"),
        QStringLiteral("mức nhiên liệu"),
        QStringLiteral("nhiệt độ máy"),
        QStringLiteral("pin còn bao nhiêu"),
        QStringLiteral("quãng đường còn lại"),
        QStringLiteral("tình trạng xe"),
        QStringLiteral("xăng còn bao nhiêu"),
        QStringLiteral("áp suất lốp"),
        QStringLiteral("đèn xe")
    };
    return values;
}

int seatLevelFromText(const QString& text)
{
    const QString normalized = normalizedForMatch(text);
    if (normalized.contains(QStringLiteral("tat"))) {
        return 0;
    }

    const int level = levelFromText(text, 0, 3);
    if (level < 0 || level > 3) {
        return -100;
    }

    if (level == 0) {
        return 0;
    }

    if (normalized.contains(QStringLiteral("mat")) ||
        normalized.contains(QStringLiteral("lanh"))) {
        return -level;
    }

    return level;
}

QString airflowModeFromActionOrText(const QString& action, const QString& text)
{
    if (action == QStringLiteral("HVAC_AIRFLOW_FACE")) return QStringLiteral("face");
    if (action == QStringLiteral("HVAC_AIRFLOW_FEET")) return QStringLiteral("feet");
    if (action == QStringLiteral("HVAC_AIRFLOW_FACE_FEET")) return QStringLiteral("face_feet");
    if (action == QStringLiteral("HVAC_AIRFLOW_FEET_DEFROST")) return QStringLiteral("feet_windshield");

    const QString normalized = normalizedForMatch(text);
    if (normalized.contains(QStringLiteral("mat")) && normalized.contains(QStringLiteral("chan"))) {
        return QStringLiteral("face_feet");
    }
    if (normalized.contains(QStringLiteral("chan")) && normalized.contains(QStringLiteral("kinh"))) {
        return QStringLiteral("feet_windshield");
    }
    if (normalized.contains(QStringLiteral("mat"))) return QStringLiteral("face");
    if (normalized.contains(QStringLiteral("chan"))) return QStringLiteral("feet");
    if (normalized.contains(QStringLiteral("kinh"))) return QStringLiteral("windshield");
    return {};
}

QString vehicleMetricFromText(const QString& text)
{
    const QString normalized = normalizedForMatch(text);
    if (normalized.contains(QStringLiteral("nhien lieu")) ||
        normalized.contains(QStringLiteral("xang")) ||
        normalized.contains(QStringLiteral("pin"))) {
        return QStringLiteral("fuel_or_battery");
    }
    if (normalized.contains(QStringLiteral("lop")) ||
        normalized.contains(QStringLiteral("ap suat"))) {
        return QStringLiteral("tire_pressure");
    }
    if (normalized.contains(QStringLiteral("toc do"))) return QStringLiteral("speed");
    if (normalized.contains(QStringLiteral("odo")) ||
        normalized.contains(QStringLiteral("quang duong"))) {
        return QStringLiteral("odometer");
    }
    return QStringLiteral("vehicle_status");
}

void insertNumberIfValid(SlotMap* slotValues,
                         const QString& name,
                         const QString& text,
                         int minValue,
                         int maxValue)
{
    const int value = levelFromText(text, minValue, maxValue);
    if (value >= minValue && value <= maxValue) {
        slotValues->insert(name, value);
    }
}

} // namespace

SlotMap PhoBertSlotExtractor::extract(const Input& input) const
{
    const QString slotText = input.originalText.trimmed().isEmpty()
        ? input.normalizedText
        : input.originalText;

    SlotMap slotValues = extractByAction(input, slotText);
    if (!slotValues.isEmpty()) {
        return slotValues;
    }

    return extractBySlotType(input, slotText);
}

SlotMap PhoBertSlotExtractor::extractByAction(const Input& input, const QString& slotText) const
{
    SlotMap slotValues;
    const QString& action = input.action;

    if (action == QStringLiteral("HVAC_SET_TEMPERATURE")) {
        int value = firstNumberInRange(slotText, 16, 30);
        if (input.modelAction == QStringLiteral("HVAC_TEMP_MAX_COLD")) value = 16;
        if (input.modelAction == QStringLiteral("HVAC_TEMP_MAX_HOT")) value = 30;
        if (value >= 16 && value <= 30) slotValues.insert(QStringLiteral("temperature"), value);
    } else if (action == QStringLiteral("HVAC_SET_FAN_LEVEL")) {
        insertNumberIfValid(&slotValues, QStringLiteral("fan_level"), slotText, 1, 7);
    } else if (action == QStringLiteral("HVAC_SEAT_DRIVER_SET_LEVEL") ||
               action == QStringLiteral("HVAC_SEAT_PASSENGER_SET_LEVEL") ||
               action == QStringLiteral("HVAC_SEAT_REAR_LEFT_SET_LEVEL") ||
               action == QStringLiteral("HVAC_SEAT_REAR_RIGHT_SET_LEVEL")) {
        const int level = seatLevelFromText(slotText);
        if (level >= -3 && level <= 3) slotValues.insert(QStringLiteral("seat_level"), level);
    } else if (action == QStringLiteral("HVAC_MODE_FACE") ||
               action == QStringLiteral("HVAC_MODE_FEET") ||
               action == QStringLiteral("HVAC_MODE_FACE_FEET") ||
               action == QStringLiteral("HVAC_MODE_FEET_WINDSHIELD") ||
               input.modelAction.startsWith(QStringLiteral("HVAC_AIRFLOW_"))) {
        const QString mode = airflowModeFromActionOrText(input.modelAction, slotText);
        if (!mode.isEmpty()) slotValues.insert(QStringLiteral("airflow_mode"), mode);
    } else if (action == QStringLiteral("MEDIA_SET_VOLUME")) {
        insertNumberIfValid(&slotValues, QStringLiteral("volume_level"), slotText, 0, 100);
    } else if (action == QStringLiteral("MEDIA_PLAY_SONG")) {
        QString song = knownSlotValueFromText(slotText, songValues());
        if (song.isEmpty()) song = afterLastKeyword(slotText, {
            QStringLiteral("mở bài hát"), QStringLiteral("phát bài hát"),
            QStringLiteral("mở bài"), QStringLiteral("phát bài"),
            QStringLiteral("bài hát"), QStringLiteral("bài"),
            QStringLiteral("phát")
        });
        if (!song.isEmpty()) slotValues.insert(QStringLiteral("song_name"), song);
    } else if (action == QStringLiteral("MEDIA_PLAY_ARTIST") ||
               action == QStringLiteral("MEDIA_LIST_BY_ARTIST")) {
        QString artist = knownSlotValueFromText(slotText, artistValues());
        if (artist.isEmpty()) artist = afterLastKeyword(slotText, {
            QStringLiteral("nghe nhạc của"), QStringLiteral("phát nhạc của"),
            QStringLiteral("mở nhạc của"), QStringLiteral("bật nhạc của"),
            QStringLiteral("nghe ca sĩ"), QStringLiteral("nghe nghệ sĩ"),
            QStringLiteral("phát ca sĩ"), QStringLiteral("mở ca sĩ"),
            QStringLiteral("phát nhạc"), QStringLiteral("mở nhạc"),
            QStringLiteral("bật nhạc"), QStringLiteral("nhạc của"),
            QStringLiteral("nghệ sĩ"), QStringLiteral("ca sĩ"),
            QStringLiteral("của"), QStringLiteral("cho nghe"),
            QStringLiteral("nghe"), QStringLiteral("mở"),
            QStringLiteral("phát"), QStringLiteral("bật")
        });
        if (!artist.isEmpty()) slotValues.insert(QStringLiteral("artist_name"), artist);
    } else if (action == QStringLiteral("PHONE_CALL_CONTACT") ||
               action == QStringLiteral("PHONE_SEARCH_CONTACT")) {
        QString contact = afterLastKeyword(slotText, {
            QStringLiteral("gọi điện thoại cho"), QStringLiteral("gọi điện cho"),
            QStringLiteral("điện thoại cho"), QStringLiteral("điện cho"),
            QStringLiteral("tìm liên hệ"), QStringLiteral("tìm số"),
            QStringLiteral("tìm"), QStringLiteral("gọi cho"), QStringLiteral("gọi")
        });
        if (contact.isEmpty()) contact = knownSlotValueFromText(slotText, contactValues());
        if (!contact.isEmpty()) slotValues.insert(QStringLiteral("contact_name"), contact);
    } else if (action == QStringLiteral("PHONE_DIAL_NUMBER")) {
        const QString digits = phoneDigitsFromText(phoneNumberSlotText(slotText));
        if (isValidPhoneNumber(digits)) {
            slotValues.insert(QStringLiteral("phone_number"), digits);
        }
    } else if (action == QStringLiteral("NAVIGATION_START")) {
        QString dest = knownSlotValueFromText(slotText, destinationValues());
        if (dest.isEmpty()) dest = afterLastKeyword(slotText, {
            QStringLiteral("dẫn đường đến"), QStringLiteral("dẫn đường tới"),
            QStringLiteral("dẫn tôi đến"), QStringLiteral("dẫn tôi tới"),
            QStringLiteral("dẫn đến"), QStringLiteral("dẫn tới"),
            QStringLiteral("chỉ đường đến"), QStringLiteral("chỉ đường tới"),
            QStringLiteral("tìm đường đến"), QStringLiteral("tìm đường tới"),
            QStringLiteral("đường đến"), QStringLiteral("đường tới"),
            QStringLiteral("đưa tôi đến"), QStringLiteral("đưa tôi tới"),
            QStringLiteral("đưa đến"), QStringLiteral("đưa tới"),
            QStringLiteral("đưa tôi ra"), QStringLiteral("dẫn đường"),
            QStringLiteral("chỉ đường"), QStringLiteral("đưa tôi"),
            QStringLiteral("đi đến"), QStringLiteral("đi tới"),
            QStringLiteral("đi về"), QStringLiteral("về"),
            QStringLiteral("đến"), QStringLiteral("tới"),
            QStringLiteral("ra"), QStringLiteral("đi")
        });
        if (!dest.isEmpty()) slotValues.insert(QStringLiteral("destination"), dest);
    } else if (action == QStringLiteral("NAVIGATION_SEARCH_POI")) {
        QString poi = knownSlotValueFromText(slotText, poiValues());
        if (poi.isEmpty()) poi = afterLastKeyword(slotText, {
            QStringLiteral("tìm kiếm"), QStringLiteral("tìm"),
            QStringLiteral("kiếm")
        });
        if (!poi.isEmpty()) slotValues.insert(QStringLiteral("poi"), poi);
    } else if (action == QStringLiteral("VEHICLE_STATUS_QUERY")) {
        QString metric = knownSlotValueFromText(slotText, vehicleMetricValues());
        if (metric.isEmpty()) metric = vehicleMetricFromText(slotText);
        slotValues.insert(QStringLiteral("vehicle_metric"), metric);
    }

    return slotValues;
}

SlotMap PhoBertSlotExtractor::extractBySlotType(const Input& input, const QString& slotText) const
{
    SlotMap slotValues;
    const QString& slotType = input.slotType;

    if (slotType == QStringLiteral("temperature")) {
        insertNumberIfValid(&slotValues, QStringLiteral("temperature"), slotText, 16, 30);
    } else if (slotType == QStringLiteral("fan_level")) {
        insertNumberIfValid(&slotValues, QStringLiteral("fan_level"), slotText, 1, 7);
    } else if (slotType == QStringLiteral("volume_level")) {
        insertNumberIfValid(&slotValues, QStringLiteral("volume_level"), slotText, 0, 100);
    } else if (slotType == QStringLiteral("seat_level")) {
        const int level = seatLevelFromText(slotText);
        if (level >= -3 && level <= 3) slotValues.insert(QStringLiteral("seat_level"), level);
    } else if (slotType == QStringLiteral("phone_number")) {
        const QString digits = phoneDigitsFromText(phoneNumberSlotText(slotText));
        if (isValidPhoneNumber(digits)) {
            slotValues.insert(QStringLiteral("phone_number"), digits);
        }
    } else if (slotType == QStringLiteral("contact_name")) {
        QString contact = afterLastKeyword(slotText, {
            QStringLiteral("gọi điện thoại cho"), QStringLiteral("gọi điện cho"),
            QStringLiteral("gọi cho"), QStringLiteral("gọi"), QStringLiteral("tìm")
        });
        if (contact.isEmpty()) contact = knownSlotValueFromText(slotText, contactValues());
        if (!contact.isEmpty()) slotValues.insert(QStringLiteral("contact_name"), contact);
    } else if (slotType == QStringLiteral("destination")) {
        QString dest = knownSlotValueFromText(slotText, destinationValues());
        if (dest.isEmpty()) dest = afterLastKeyword(slotText, {
            QStringLiteral("dẫn đường đến"), QStringLiteral("dẫn đường tới"),
            QStringLiteral("dẫn tôi đến"), QStringLiteral("dẫn tôi tới"),
            QStringLiteral("dẫn đến"), QStringLiteral("dẫn tới"),
            QStringLiteral("chỉ đường đến"), QStringLiteral("chỉ đường tới"),
            QStringLiteral("tìm đường đến"), QStringLiteral("tìm đường tới"),
            QStringLiteral("đường đến"), QStringLiteral("đường tới"),
            QStringLiteral("đưa tôi đến"), QStringLiteral("đưa tôi tới"),
            QStringLiteral("đưa đến"), QStringLiteral("đưa tới"),
            QStringLiteral("đi đến"), QStringLiteral("đi tới"),
            QStringLiteral("đi về"), QStringLiteral("về"),
            QStringLiteral("đến"), QStringLiteral("tới"), QStringLiteral("ra")
        });
        if (!dest.isEmpty()) slotValues.insert(QStringLiteral("destination"), dest);
    } else if (slotType == QStringLiteral("poi")) {
        QString poi = knownSlotValueFromText(slotText, poiValues());
        if (poi.isEmpty()) poi = afterLastKeyword(slotText, {
            QStringLiteral("tìm kiếm"), QStringLiteral("tìm"), QStringLiteral("kiếm")
        });
        if (!poi.isEmpty()) slotValues.insert(QStringLiteral("poi"), poi);
    } else if (slotType == QStringLiteral("song_name")) {
        QString song = knownSlotValueFromText(slotText, songValues());
        if (song.isEmpty()) song = afterLastKeyword(slotText, {
            QStringLiteral("mở bài hát"), QStringLiteral("phát bài hát"),
            QStringLiteral("mở bài"), QStringLiteral("phát bài"), QStringLiteral("bài")
        });
        if (!song.isEmpty()) slotValues.insert(QStringLiteral("song_name"), song);
    } else if (slotType == QStringLiteral("artist_name")) {
        QString artist = knownSlotValueFromText(slotText, artistValues());
        if (artist.isEmpty()) artist = afterLastKeyword(slotText, {
            QStringLiteral("nghe nhạc của"), QStringLiteral("phát nhạc của"),
            QStringLiteral("mở nhạc của"), QStringLiteral("bật nhạc của"),
            QStringLiteral("nghe ca sĩ"), QStringLiteral("nghe nghệ sĩ"),
            QStringLiteral("phát ca sĩ"), QStringLiteral("mở ca sĩ"),
            QStringLiteral("phát nhạc"), QStringLiteral("mở nhạc"),
            QStringLiteral("bật nhạc"), QStringLiteral("nhạc của"),
            QStringLiteral("nghệ sĩ"), QStringLiteral("ca sĩ"),
            QStringLiteral("của"), QStringLiteral("cho nghe"),
            QStringLiteral("nghe"), QStringLiteral("mở"),
            QStringLiteral("phát"), QStringLiteral("bật")
        });
        if (!artist.isEmpty()) slotValues.insert(QStringLiteral("artist_name"), artist);
    } else if (slotType == QStringLiteral("airflow_mode")) {
        const QString mode = airflowModeFromActionOrText(input.modelAction, slotText);
        if (!mode.isEmpty()) slotValues.insert(QStringLiteral("airflow_mode"), mode);
    } else if (slotType == QStringLiteral("vehicle_metric")) {
        QString metric = knownSlotValueFromText(slotText, vehicleMetricValues());
        if (metric.isEmpty()) metric = vehicleMetricFromText(slotText);
        slotValues.insert(QStringLiteral("vehicle_metric"), metric);
    }

    return slotValues;
}

} // namespace nlu
