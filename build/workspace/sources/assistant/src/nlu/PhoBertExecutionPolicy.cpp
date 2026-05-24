#include "PhoBertExecutionPolicy.h"

#include <QMap>
#include <QMetaType>
#include <QRegularExpression>
#include <QSet>
#include <QVariant>

namespace nlu {

namespace {

QString stripVietnameseDiacritics(QString text)
{
    static const QMap<QString, QString> replacements = {
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

    text = text.toLower().trimmed();
    text.replace(QRegularExpression(QStringLiteral("[\\.,;:!?\\[\\]\\(\\)\\{\\}\\\"']+")),
                 QStringLiteral(" "));
    text.replace(QRegularExpression(QStringLiteral("\\s+")), QStringLiteral(" "));
    for (auto it = replacements.constBegin(); it != replacements.constEnd(); ++it) {
        text.replace(it.key(), it.value(), Qt::CaseInsensitive);
    }
    return text.trimmed();
}

bool containsWord(const QString& text, const QString& word)
{
    const QRegularExpression re(
        QStringLiteral("(^|\\s)%1(\\s|$)").arg(QRegularExpression::escape(word)),
        QRegularExpression::CaseInsensitiveOption
    );
    return text.contains(re);
}

} // namespace

QString PhoBertExecutionPolicy::normalizeModelAction(const QString& action)
{
    static const QMap<QString, QString> aliases = {
        {QStringLiteral("HVAC_AIRFLOW_FACE"), QStringLiteral("HVAC_MODE_FACE")},
        {QStringLiteral("HVAC_AIRFLOW_FEET"), QStringLiteral("HVAC_MODE_FEET")},
        {QStringLiteral("HVAC_AIRFLOW_FEET_DEFROST"), QStringLiteral("HVAC_MODE_FEET_WINDSHIELD")},
        {QStringLiteral("HVAC_AIRFLOW_FACE_FEET"), QStringLiteral("HVAC_MODE_FACE_FEET")},
        {QStringLiteral("HVAC_AIR_SOURCE_FRESH_AIR"), QStringLiteral("HVAC_SET_AIR_SOURCE_FRESH_AIR")},
        {QStringLiteral("HVAC_AIR_SOURCE_RECIRCULATION"), QStringLiteral("HVAC_SET_AIR_SOURCE_RECIRCULATION")},
        {QStringLiteral("HVAC_FRONT_DEFROST_ON"), QStringLiteral("HVAC_DEFROST_FRONT_ON")},
        {QStringLiteral("HVAC_FRONT_DEFROST_OFF"), QStringLiteral("HVAC_DEFROST_FRONT_OFF")},
        {QStringLiteral("HVAC_REAR_DEFROST_ON"), QStringLiteral("HVAC_DEFROST_REAR_ON")},
        {QStringLiteral("HVAC_REAR_DEFROST_OFF"), QStringLiteral("HVAC_DEFROST_REAR_OFF")},
        {QStringLiteral("HVAC_FAN_SET_LEVEL"), QStringLiteral("HVAC_SET_FAN_LEVEL")},
        {QStringLiteral("HVAC_TEMP_MAX_COLD"), QStringLiteral("HVAC_SET_TEMPERATURE")},
        {QStringLiteral("HVAC_TEMP_MAX_HOT"), QStringLiteral("HVAC_SET_TEMPERATURE")},
        {QStringLiteral("PHONE_ACCEPT"), QStringLiteral("PHONE_ACCEPT_CALL")},
        {QStringLiteral("PHONE_DIAL_NUMBER"), QStringLiteral("PHONE_DIAL_NUMBER")},
        {QStringLiteral("PHONE_END_CALL"), QStringLiteral("PHONE_HANGUP")},
        {QStringLiteral("PHONE_CALL_BACK"), QStringLiteral("PHONE_CALL_BACK")},
        {QStringLiteral("PHONE_MUTE_MIC"), QStringLiteral("PHONE_MUTE_MIC")},
        {QStringLiteral("PHONE_OPEN_CONTACTS"), QStringLiteral("PHONE_OPEN_CONTACTS")},
        {QStringLiteral("PHONE_REDIAL_LAST"), QStringLiteral("PHONE_REDIAL_LAST")},
        {QStringLiteral("PHONE_SEARCH_CONTACT"), QStringLiteral("PHONE_SEARCH_CONTACT")},
        {QStringLiteral("PHONE_SPEAKER_OFF"), QStringLiteral("PHONE_SPEAKER_OFF")},
        {QStringLiteral("PHONE_SPEAKER_ON"), QStringLiteral("PHONE_SPEAKER_ON")},
        {QStringLiteral("PHONE_UNMUTE_MIC"), QStringLiteral("PHONE_UNMUTE_MIC")},
        {QStringLiteral("NAVIGATION_CANCEL"), QStringLiteral("NAVIGATION_STOP")}
    };

    return aliases.value(action, action);
}

QString PhoBertExecutionPolicy::normalizeModelAction(const QString& action,
                                                     const QString& target,
                                                     const QString& slotType)
{
    if ((target == QStringLiteral("ARTIST") ||
         slotType == QStringLiteral("artist_name")) &&
        (action == QStringLiteral("MEDIA_PLAY") ||
         action == QStringLiteral("MEDIA_PLAY_SONG"))) {
        return QStringLiteral("MEDIA_PLAY_ARTIST");
    }

    if ((target == QStringLiteral("SONG") ||
         slotType == QStringLiteral("song_name")) &&
        action == QStringLiteral("MEDIA_PLAY")) {
        return QStringLiteral("MEDIA_PLAY_SONG");
    }

    if ((target == QStringLiteral("DESTINATION") ||
         slotType == QStringLiteral("destination")) &&
        action == QStringLiteral("NAVIGATION_SEARCH_POI")) {
        return QStringLiteral("NAVIGATION_START");
    }

    if ((target == QStringLiteral("POI") ||
         slotType == QStringLiteral("poi")) &&
        action == QStringLiteral("NAVIGATION_START")) {
        return QStringLiteral("NAVIGATION_SEARCH_POI");
    }

    if ((target == QStringLiteral("PHONE_NUMBER") ||
         slotType == QStringLiteral("phone_number")) &&
        (action == QStringLiteral("PHONE_CALL_CONTACT") ||
         action == QStringLiteral("PHONE_DIAL_NUMBER"))) {
        return QStringLiteral("PHONE_DIAL_NUMBER");
    }

    return normalizeModelAction(action);
}

ResolvedAction PhoBertExecutionPolicy::apply(const Input& input) const
{
    ResolvedAction resolved;

    resolved.matched = true;
    resolved.normalizedText = input.normalizedText;
    resolved.domain = input.domain;
    resolved.op = input.op;
    resolved.target = input.target;
    resolved.slotType = input.slotType;
    resolved.confidence = input.confidence;
    resolved.action = input.action;
    resolved.slotValues = input.slotValues;
    resolved.evidence = {QStringLiteral("phobert_multihead:%1").arg(input.modelAction)};

    if ((looksLikeGeneralQuestion(input.normalizedText) && !isServiceQueryAction(input.action)) ||
        input.execute == QStringLiteral("FALLBACK_LLM") ||
        input.modelAction == QStringLiteral("GENERAL_QA")) {
        resolved.shouldExecute = false;
        resolved.decision = QStringLiteral("LLM_QUERY");
        resolved.reply = QStringLiteral("Tôi sẽ trả lời bằng mô hình ngôn ngữ.");
        return resolved;
    }

    if (input.execute == QStringLiteral("NO_EXECUTE") ||
        input.modelAction == QStringLiteral("UNKNOWN") ||
        input.modelAction == QStringLiteral("NO_ACTION") ||
        input.modelAction.endsWith(QStringLiteral("_FRAGMENT_OR_UNKNOWN")) ||
        input.modelAction.endsWith(QStringLiteral("_VALUE_FRAGMENT"))) {
        resolved.shouldExecute = false;
        resolved.decision = QStringLiteral("ASK_CLARIFY");
        resolved.reply = QStringLiteral("Tôi chưa rõ lệnh này, bạn nói lại cụ thể hơn giúp tôi.");
        return resolved;
    }

    if (!isKnownExecutableServiceAction(input.action)) {
        resolved.shouldExecute = false;
        resolved.decision = QStringLiteral("NO_ACTION");
        resolved.reply = QStringLiteral("Tôi đã nhận ra lệnh %1 nhưng chức năng này chưa được nối service.")
                             .arg(input.modelAction);
        return resolved;
    }

    const QString requiredSlot = requiredSlotForAction(input.action);
    if (!requiredSlot.isEmpty() && !hasUsableSlotValue(input.slotValues, requiredSlot)) {
        resolved.shouldExecute = false;
        resolved.decision = QStringLiteral("ASK_SLOT");
        resolved.reply = missingSlotReply(requiredSlot);
        resolved.evidence.push_back(QStringLiteral("missing_slot:%1").arg(requiredSlot));
        return resolved;
    }

    resolved.shouldExecute = true;
    resolved.decision = QStringLiteral("EXECUTE");
    resolved.reply = replyForAction(input.action, input.slotValues);
    return resolved;
}

bool PhoBertExecutionPolicy::hasUsableSlotValue(const SlotMap& slotValues, const QString& slotName)
{
    const QVariant value = slotValues.value(slotName);
    if (!value.isValid()) return false;
    if (value.typeId() == QMetaType::Int) return true;
    return !value.toString().trimmed().isEmpty();
}

bool PhoBertExecutionPolicy::isServiceQueryAction(const QString& action)
{
    return action == QStringLiteral("HVAC_STATUS_QUERY") ||
           action == QStringLiteral("VEHICLE_STATUS_QUERY");
}

bool PhoBertExecutionPolicy::startsWithExplicitCommand(const QString& text)
{
    static const QStringList commandPrefixes = {
        QStringLiteral("bat"), QStringLiteral("tat"), QStringLiteral("mo"),
        QStringLiteral("dong"), QStringLiteral("goi"), QStringLiteral("alo"),
        QStringLiteral("bam"), QStringLiteral("quay so"), QStringLiteral("tim"),
        QStringLiteral("kiem"), QStringLiteral("dan duong"), QStringLiteral("chi duong"),
        QStringLiteral("di toi"), QStringLiteral("dua toi"), QStringLiteral("phat"),
        QStringLiteral("nghe"), QStringLiteral("chinh"), QStringLiteral("dat"),
        QStringLiteral("set"), QStringLiteral("tang"), QStringLiteral("giam"),
        QStringLiteral("cho quat"), QStringLiteral("cho gio")
    };

    for (const QString& prefix : commandPrefixes) {
        if (text == prefix || text.startsWith(prefix + QStringLiteral(" "))) {
            return true;
        }
    }
    return false;
}

bool PhoBertExecutionPolicy::looksLikeGeneralQuestion(const QString& text)
{
    const QString normalized = stripVietnameseDiacritics(text);
    if (normalized.isEmpty() || startsWithExplicitCommand(normalized)) {
        return false;
    }

    if (containsWord(normalized, QStringLiteral("la gi")) ||
        normalized.contains(QStringLiteral(" la ai")) ||
        normalized.startsWith(QStringLiteral("ai la ")) ||
        normalized.contains(QStringLiteral("ai la ")) ||
        normalized.contains(QStringLiteral("nghia la gi")) ||
        normalized.contains(QStringLiteral("dinh nghia")) ||
        normalized.contains(QStringLiteral("giai thich")) ||
        normalized.contains(QStringLiteral("tai sao")) ||
        normalized.contains(QStringLiteral("vi sao")) ||
        normalized.contains(QStringLiteral("nhu the nao")) ||
        normalized.contains(QStringLiteral("the nao")) ||
        normalized.contains(QStringLiteral("bao nhieu")) ||
        normalized.contains(QStringLiteral("khi nao")) ||
        normalized.contains(QStringLiteral("o dau")) ||
        normalized.contains(QStringLiteral("la ai"))) {
        return true;
    }

    return false;
}

QString PhoBertExecutionPolicy::requiredSlotForAction(const QString& action)
{
    if (action == QStringLiteral("HVAC_SET_TEMPERATURE")) return QStringLiteral("temperature");
    if (action == QStringLiteral("HVAC_SET_FAN_LEVEL")) return QStringLiteral("fan_level");
    if (action == QStringLiteral("HVAC_SEAT_DRIVER_SET_LEVEL")) return QStringLiteral("seat_level");
    if (action == QStringLiteral("HVAC_SEAT_PASSENGER_SET_LEVEL")) return QStringLiteral("seat_level");
    if (action == QStringLiteral("HVAC_SEAT_REAR_LEFT_SET_LEVEL")) return QStringLiteral("seat_level");
    if (action == QStringLiteral("HVAC_SEAT_REAR_RIGHT_SET_LEVEL")) return QStringLiteral("seat_level");
    if (action == QStringLiteral("MEDIA_SET_VOLUME")) return QStringLiteral("volume_level");
    if (action == QStringLiteral("MEDIA_PLAY_SONG")) return QStringLiteral("song_name");
    if (action == QStringLiteral("MEDIA_PLAY_ARTIST")) return QStringLiteral("artist_name");
    if (action == QStringLiteral("MEDIA_LIST_BY_ARTIST")) return QStringLiteral("artist_name");
    if (action == QStringLiteral("PHONE_CALL_CONTACT")) return QStringLiteral("contact_name");
    if (action == QStringLiteral("PHONE_SEARCH_CONTACT")) return QStringLiteral("contact_name");
    if (action == QStringLiteral("PHONE_DIAL_NUMBER")) return QStringLiteral("phone_number");
    if (action == QStringLiteral("NAVIGATION_START")) return QStringLiteral("destination");
    if (action == QStringLiteral("NAVIGATION_SEARCH_POI")) return QStringLiteral("poi");
    if (action == QStringLiteral("VEHICLE_STATUS_QUERY")) return QStringLiteral("vehicle_metric");
    return {};
}

QString PhoBertExecutionPolicy::missingSlotReply(const QString& slotName)
{
    if (slotName == QStringLiteral("temperature")) return QStringLiteral("Bạn muốn chỉnh điều hòa bao nhiêu độ?");
    if (slotName == QStringLiteral("fan_level")) return QStringLiteral("Bạn muốn chỉnh quạt gió mức mấy?");
    if (slotName == QStringLiteral("seat_level")) return QStringLiteral("Bạn muốn chỉnh sưởi ghế mức mấy?");
    if (slotName == QStringLiteral("volume_level")) return QStringLiteral("Bạn muốn chỉnh âm lượng mức bao nhiêu?");
    if (slotName == QStringLiteral("song_name")) return QStringLiteral("Bạn muốn mở bài hát nào?");
    if (slotName == QStringLiteral("artist_name")) return QStringLiteral("Bạn muốn nghe nghệ sĩ nào?");
    if (slotName == QStringLiteral("contact_name")) return QStringLiteral("Bạn muốn gọi cho ai?");
    if (slotName == QStringLiteral("phone_number")) return QStringLiteral("Bạn muốn gọi số điện thoại nào?");
    if (slotName == QStringLiteral("destination")) return QStringLiteral("Bạn muốn dẫn đường tới đâu?");
    if (slotName == QStringLiteral("poi")) return QStringLiteral("Bạn muốn tìm địa điểm nào?");
    if (slotName == QStringLiteral("vehicle_metric")) return QStringLiteral("Bạn muốn xem thông tin nào của xe?");
    return QStringLiteral("Bạn nói rõ thêm giá trị cần điều khiển giúp tôi.");
}

bool PhoBertExecutionPolicy::isKnownExecutableServiceAction(const QString& action)
{
    static const QSet<QString> supported = {
        QStringLiteral("HVAC_TURN_ON"),
        QStringLiteral("HVAC_TURN_OFF"),
        QStringLiteral("HVAC_AUTO_ON"),
        QStringLiteral("HVAC_AUTO_OFF"),
        QStringLiteral("HVAC_SET_TEMPERATURE"),
        QStringLiteral("HVAC_TEMP_DOWN"),
        QStringLiteral("HVAC_TEMP_UP"),
        QStringLiteral("HVAC_FAN_UP"),
        QStringLiteral("HVAC_FAN_DOWN"),
        QStringLiteral("HVAC_SET_FAN_LEVEL"),
        QStringLiteral("HVAC_SET_AIR_SOURCE_RECIRCULATION"),
        QStringLiteral("HVAC_SET_AIR_SOURCE_FRESH_AIR"),
        QStringLiteral("HVAC_DEFROST_FRONT_ON"),
        QStringLiteral("HVAC_DEFROST_FRONT_OFF"),
        QStringLiteral("HVAC_DEFROST_REAR_ON"),
        QStringLiteral("HVAC_DEFROST_REAR_OFF"),
        QStringLiteral("HVAC_SYNC_ON"),
        QStringLiteral("HVAC_SYNC_OFF"),
        QStringLiteral("HVAC_MODE_FACE"),
        QStringLiteral("HVAC_MODE_FEET"),
        QStringLiteral("HVAC_MODE_WINDSHIELD"),
        QStringLiteral("HVAC_MODE_FACE_FEET"),
        QStringLiteral("HVAC_MODE_FEET_WINDSHIELD"),
        QStringLiteral("HVAC_SEAT_DRIVER_SET_LEVEL"),
        QStringLiteral("HVAC_SEAT_PASSENGER_SET_LEVEL"),
        QStringLiteral("HVAC_SEAT_REAR_LEFT_SET_LEVEL"),
        QStringLiteral("HVAC_SEAT_REAR_RIGHT_SET_LEVEL"),
        QStringLiteral("HVAC_STATUS_QUERY"),
        QStringLiteral("MEDIA_PLAY"),
        QStringLiteral("MEDIA_STOP"),
        QStringLiteral("MEDIA_NEXT_TRACK"),
        QStringLiteral("MEDIA_PREVIOUS_TRACK"),
        QStringLiteral("MEDIA_VOLUME_UP"),
        QStringLiteral("MEDIA_VOLUME_DOWN"),
        QStringLiteral("MEDIA_SET_VOLUME"),
        QStringLiteral("MEDIA_MUTE"),
        QStringLiteral("MEDIA_UNMUTE"),
        QStringLiteral("MEDIA_PLAY_SONG"),
        QStringLiteral("MEDIA_PLAY_ARTIST"),
        QStringLiteral("MEDIA_LIST_BY_ARTIST"),
        QStringLiteral("PHONE_CALL_BACK"),
        QStringLiteral("PHONE_CALL_CONTACT"),
        QStringLiteral("PHONE_DIAL_NUMBER"),
        QStringLiteral("PHONE_HANGUP"),
        QStringLiteral("PHONE_ACCEPT_CALL"),
        QStringLiteral("PHONE_MUTE_MIC"),
        QStringLiteral("PHONE_OPEN_CONTACTS"),
        QStringLiteral("PHONE_REDIAL_LAST"),
        QStringLiteral("PHONE_SEARCH_CONTACT"),
        QStringLiteral("PHONE_SPEAKER_OFF"),
        QStringLiteral("PHONE_SPEAKER_ON"),
        QStringLiteral("PHONE_SYNC_CONTACTS"),
        QStringLiteral("PHONE_UNMUTE_MIC"),
        QStringLiteral("NAVIGATION_START"),
        QStringLiteral("NAVIGATION_SEARCH_POI"),
        QStringLiteral("NAVIGATION_STOP"),
        QStringLiteral("NAVIGATION_HOME"),
        QStringLiteral("NAVIGATION_WORK"),
        QStringLiteral("VEHICLE_STATUS_QUERY")
    };

    return supported.contains(action);
}

QString PhoBertExecutionPolicy::replyForAction(const QString& action, const SlotMap& slotValues)
{
    if (action == QStringLiteral("HVAC_TURN_ON")) return QStringLiteral("Đã bật điều hòa.");
    if (action == QStringLiteral("HVAC_TURN_OFF")) return QStringLiteral("Đã tắt điều hòa.");
    if (action == QStringLiteral("HVAC_AUTO_ON")) return QStringLiteral("Đã bật chế độ điều hòa tự động.");
    if (action == QStringLiteral("HVAC_AUTO_OFF")) return QStringLiteral("Đã tắt chế độ điều hòa tự động.");
    if (action == QStringLiteral("HVAC_TEMP_DOWN")) return QStringLiteral("Đã giảm nhiệt độ.");
    if (action == QStringLiteral("HVAC_TEMP_UP")) return QStringLiteral("Đã tăng nhiệt độ.");
    if (action == QStringLiteral("HVAC_SET_TEMPERATURE")) {
        return QStringLiteral("Đã chỉnh điều hòa về %1 độ.")
            .arg(slotValues.value(QStringLiteral("temperature")).toString());
    }
    if (action == QStringLiteral("HVAC_FAN_UP")) return QStringLiteral("Đã tăng quạt gió.");
    if (action == QStringLiteral("HVAC_FAN_DOWN")) return QStringLiteral("Đã giảm quạt gió.");
    if (action == QStringLiteral("HVAC_SET_FAN_LEVEL")) {
        return QStringLiteral("Đã chỉnh quạt gió mức %1.")
            .arg(slotValues.value(QStringLiteral("fan_level")).toString());
    }
    if (action == QStringLiteral("HVAC_SET_AIR_SOURCE_RECIRCULATION")) return QStringLiteral("Đã chuyển sang chế độ lấy gió trong.");
    if (action == QStringLiteral("HVAC_SET_AIR_SOURCE_FRESH_AIR")) return QStringLiteral("Đã chuyển sang chế độ lấy gió ngoài.");
    if (action == QStringLiteral("HVAC_DEFROST_FRONT_ON")) return QStringLiteral("Đã bật sưởi kính trước.");
    if (action == QStringLiteral("HVAC_DEFROST_FRONT_OFF")) return QStringLiteral("Đã tắt sưởi kính trước.");
    if (action == QStringLiteral("HVAC_DEFROST_REAR_ON")) return QStringLiteral("Đã bật sưởi kính sau.");
    if (action == QStringLiteral("HVAC_DEFROST_REAR_OFF")) return QStringLiteral("Đã tắt sưởi kính sau.");
    if (action == QStringLiteral("HVAC_MODE_FACE")) return QStringLiteral("Đã chuyển hướng gió thổi vào mặt.");
    if (action == QStringLiteral("HVAC_MODE_FEET")) return QStringLiteral("Đã chuyển hướng gió thổi xuống chân.");
    if (action == QStringLiteral("HVAC_MODE_WINDSHIELD")) return QStringLiteral("Đã chuyển hướng gió thổi lên kính.");
    if (action == QStringLiteral("HVAC_MODE_FACE_FEET")) return QStringLiteral("Đã chuyển hướng gió thổi vào mặt và xuống chân.");
    if (action == QStringLiteral("HVAC_MODE_FEET_WINDSHIELD")) return QStringLiteral("Đã chuyển hướng gió thổi xuống chân và lên kính.");
    // if (action == QStringLiteral("HVAC_SEAT_DRIVER_SET_LEVEL")) {
    //     return QStringLiteral("Đã chỉnh ghế lái mức %1.")
    //         .arg(slotValues.value(QStringLiteral("seat_level")).toString());
    // }
    // if (action == QStringLiteral("HVAC_SEAT_PASSENGER_SET_LEVEL")) {
    //     return QStringLiteral("Đã chỉnh ghế phụ mức %1.")
    //         .arg(slotValues.value(QStringLiteral("seat_level")).toString());
    // }
    // if (action == QStringLiteral("HVAC_SEAT_REAR_LEFT_SET_LEVEL")) {
    //     return QStringLiteral("Đã chỉnh ghế sau trái mức %1.")
    //         .arg(slotValues.value(QStringLiteral("seat_level")).toString());
    // }
    // if (action == QStringLiteral("HVAC_SEAT_REAR_RIGHT_SET_LEVEL")) {
    //     return QStringLiteral("Đã chỉnh ghế sau phải mức %1.")
    //         .arg(slotValues.value(QStringLiteral("seat_level")).toString());
    // }

    auto seatReply = [&](const QString& seatName) -> QString {
        bool ok = false;
        int level = slotValues.value(QStringLiteral("seat_level")).toInt(&ok);

        if (!ok) {
            return QStringLiteral("Không xác định được mức ghế %1.").arg(seatName);
        }

        if (level >= -3 && level <= -1) {
            return QStringLiteral("Đã bật làm mát ghế %1 mức %2.")
                .arg(seatName)
                .arg(qAbs(level));
        }

        if (level >= 1 && level <= 3) {
            return QStringLiteral("Đã bật sưởi ghế %1 mức %2.")
                .arg(seatName)
                .arg(level);
        }

        if (level == 0) {
            return QStringLiteral("Đã chỉnh ghế %1 về mức 0.")
                .arg(seatName);
        }

        return QStringLiteral("Mức ghế %1 không hợp lệ.").arg(seatName);
    };

    if (action == QStringLiteral("HVAC_SEAT_DRIVER_SET_LEVEL")) {
        return seatReply(QStringLiteral("lái"));
    }

    if (action == QStringLiteral("HVAC_SEAT_PASSENGER_SET_LEVEL")) {
        return seatReply(QStringLiteral("phụ"));
    }

    if (action == QStringLiteral("HVAC_SEAT_REAR_LEFT_SET_LEVEL")) {
        return seatReply(QStringLiteral("sau bên trái"));
    }

    if (action == QStringLiteral("HVAC_SEAT_REAR_RIGHT_SET_LEVEL")) {
        return seatReply(QStringLiteral("sau bên phải"));
    }

    if (action == QStringLiteral("HVAC_STATUS_QUERY")) return QStringLiteral("Đang kiểm tra trạng thái điều hòa.");
    if (action == QStringLiteral("MEDIA_PLAY")) return QStringLiteral("Đã mở nhạc.");
    if (action == QStringLiteral("MEDIA_STOP")) return QStringLiteral("Đã dừng nhạc.");
    if (action == QStringLiteral("MEDIA_NEXT_TRACK")) return QStringLiteral("Đã chuyển bài tiếp theo.");
    if (action == QStringLiteral("MEDIA_PREVIOUS_TRACK")) return QStringLiteral("Đã quay lại bài trước.");
    if (action == QStringLiteral("MEDIA_VOLUME_UP")) return QStringLiteral("Đã tăng âm lượng.");
    if (action == QStringLiteral("MEDIA_VOLUME_DOWN")) return QStringLiteral("Đã giảm âm lượng.");
    if (action == QStringLiteral("MEDIA_SET_VOLUME")) {
        return QStringLiteral("Đã chỉnh âm lượng mức %1.")
            .arg(slotValues.value(QStringLiteral("volume_level")).toString());
    }
    if (action == QStringLiteral("MEDIA_MUTE")) return QStringLiteral("Đã tắt tiếng.");
    if (action == QStringLiteral("MEDIA_UNMUTE")) return QStringLiteral("Đã bật lại âm thanh.");
    if (action == QStringLiteral("MEDIA_PLAY_SONG")) {
        return QStringLiteral("Đang mở bài %1.")
            .arg(slotValues.value(QStringLiteral("song_name")).toString());
    }
    if (action == QStringLiteral("MEDIA_PLAY_ARTIST") ||
        action == QStringLiteral("MEDIA_LIST_BY_ARTIST")) {
        return QStringLiteral("Đang mở nhạc của %1.")
            .arg(slotValues.value(QStringLiteral("artist_name")).toString());
    }
    if (action == QStringLiteral("PHONE_CALL_CONTACT")) {
        return QStringLiteral("Đang gọi cho %1.")
            .arg(slotValues.value(QStringLiteral("contact_name")).toString());
    }
    if (action == QStringLiteral("PHONE_DIAL_NUMBER")) return QStringLiteral("Đang gọi số điện thoại.");
    if (action == QStringLiteral("PHONE_CALL_BACK")) return QStringLiteral("Đang gọi lại.");
    if (action == QStringLiteral("PHONE_HANGUP")) return QStringLiteral("Đã kết thúc cuộc gọi.");
    if (action == QStringLiteral("PHONE_ACCEPT_CALL")) return QStringLiteral("Đã nghe máy.");
    if (action == QStringLiteral("PHONE_MUTE_MIC")) return QStringLiteral("Đã tắt micro.");
    if (action == QStringLiteral("PHONE_OPEN_CONTACTS")) return QStringLiteral("Đã mở danh bạ.");
    if (action == QStringLiteral("PHONE_REDIAL_LAST")) return QStringLiteral("Đang gọi lại số gần nhất.");
    if (action == QStringLiteral("PHONE_SEARCH_CONTACT")) {
        return QStringLiteral("Đang tìm liên hệ %1.")
            .arg(slotValues.value(QStringLiteral("contact_name")).toString());
    }
    if (action == QStringLiteral("PHONE_SPEAKER_OFF")) return QStringLiteral("Đã tắt loa ngoài.");
    if (action == QStringLiteral("PHONE_SPEAKER_ON")) return QStringLiteral("Đã bật loa ngoài.");
    if (action == QStringLiteral("PHONE_SYNC_CONTACTS")) return QStringLiteral("Đã đồng bộ danh bạ.");
    if (action == QStringLiteral("PHONE_UNMUTE_MIC")) return QStringLiteral("Đã bật lại micro.");
    if (action == QStringLiteral("NAVIGATION_HOME")) return QStringLiteral("Đang bắt đầu dẫn đường về nhà.");
    if (action == QStringLiteral("NAVIGATION_START")) {
        return QStringLiteral("Bạn vui lòng chọn điểm đến chính xác.")
            .arg(slotValues.value(QStringLiteral("destination")).toString());
    }
    if (action == QStringLiteral("NAVIGATION_SEARCH_POI")) {
        return QStringLiteral("Đang tìm %1 gần đây.")
            .arg(slotValues.value(QStringLiteral("poi")).toString());
    }
    if (action == QStringLiteral("NAVIGATION_STOP")) return QStringLiteral("Đã dừng dẫn đường.");
    if (action == QStringLiteral("VEHICLE_STATUS_QUERY")) {
        return QStringLiteral("Đang kiểm tra %1.")
            .arg(slotValues.value(QStringLiteral("vehicle_metric")).toString());
    }

    return QStringLiteral("Đã nhận lệnh.");
}

} // namespace nlu
