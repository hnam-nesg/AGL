#include "PhoneService.h"

#include <QDebug>
#include <QMap>
#include <QRegularExpression>
#include <QStringList>

namespace {

bool isValidPhoneNumber(const QString& number)
{
    if (number.size() != 10 || !number.startsWith(QStringLiteral("0"))) {
        return false;
    }

    for (const QChar& ch : number) {
        if (!ch.isDigit()) {
            return false;
        }
    }

    return true;
}

void appendUnique(QStringList* values, const QString& value)
{
    const QString cleaned = value.simplified();
    if (!cleaned.isEmpty() && !values->contains(cleaned)) {
        values->push_back(cleaned);
    }
}

QString titleCaseWords(const QString& value)
{
    QStringList words = value.toLower().simplified().split(QRegularExpression(QStringLiteral("\\s+")),
                                                           Qt::SkipEmptyParts);
    for (QString& word : words) {
        if (!word.isEmpty()) {
            word[0] = word[0].toUpper();
        }
    }
    return words.join(QStringLiteral(" "));
}

QString contactTitleCase(QString value)
{
    value = value.toLower().simplified();

    static const QStringList relationPrefixes = {
        QStringLiteral("anh"), QStringLiteral("chi"), QStringLiteral("chị"),
        QStringLiteral("em"), QStringLiteral("ban"), QStringLiteral("bạn"),
        QStringLiteral("co"), QStringLiteral("cô"), QStringLiteral("sep"),
        QStringLiteral("sếp"), QStringLiteral("thang"), QStringLiteral("thằng"),
        QStringLiteral("bac si"), QStringLiteral("bác sĩ"), QStringLiteral("ba"),
        QStringLiteral("bà"), QStringLiteral("ong"), QStringLiteral("ông")
    };

    for (const QString& prefix : relationPrefixes) {
        if (value.startsWith(prefix + QStringLiteral(" "))) {
            return prefix + QStringLiteral(" ") + titleCaseWords(value.mid(prefix.size()).trimmed());
        }
    }

    return titleCaseWords(value);
}

QStringList contactNameCandidates(const QString& rawName)
{
    QStringList candidates;
    const QString name = rawName.simplified();

    appendUnique(&candidates, name);
    appendUnique(&candidates, contactTitleCase(name));
    appendUnique(&candidates, titleCaseWords(name));

    const QString lower = name.toLower().simplified();
    static const QMap<QString, QStringList> aliases = {
        {QStringLiteral("nam"), {QStringLiteral("Nam"), QStringLiteral("anh Nam"), QStringLiteral("Anh Nam")}},
        {QStringLiteral("minh"), {QStringLiteral("Minh"), QStringLiteral("bác sĩ Minh")}},
        {QStringLiteral("lan"), {QStringLiteral("Lan"), QStringLiteral("chị Lan"), QStringLiteral("Chị Lan")}},
        {QStringLiteral("long"), {QStringLiteral("Long"), QStringLiteral("bạn Long"), QStringLiteral("Bạn Long")}},
        {QStringLiteral("hanh"), {QStringLiteral("Hạnh"), QStringLiteral("cô Hạnh"), QStringLiteral("Cô Hạnh")}},
        {QStringLiteral("hạnh"), {QStringLiteral("Hạnh"), QStringLiteral("cô Hạnh"), QStringLiteral("Cô Hạnh")}},
        {QStringLiteral("trang"), {QStringLiteral("Trang"), QStringLiteral("em Trang"), QStringLiteral("Em Trang")}},
        {QStringLiteral("hung"), {QStringLiteral("Hùng"), QStringLiteral("sếp Hùng"), QStringLiteral("Sếp Hùng")}},
        {QStringLiteral("hùng"), {QStringLiteral("Hùng"), QStringLiteral("sếp Hùng"), QStringLiteral("Sếp Hùng")}},
        {QStringLiteral("tuan"), {QStringLiteral("Tuấn"), QStringLiteral("thằng Tuấn"), QStringLiteral("Thằng Tuấn")}},
        {QStringLiteral("tuấn"), {QStringLiteral("Tuấn"), QStringLiteral("thằng Tuấn"), QStringLiteral("Thằng Tuấn")}},
        {QStringLiteral("bao"), {QStringLiteral("Bảo"), QStringLiteral("Quốc Bảo")}},
        {QStringLiteral("bảo"), {QStringLiteral("Bảo"), QStringLiteral("Quốc Bảo")}}
    };

    for (const QString& alias : aliases.value(lower)) {
        appendUnique(&candidates, alias);
    }

    return candidates;
}

} // namespace

bool PhoneService::executeAction(const QString& action, const nlu::SlotMap& slotValues)
{
    qInfo() << "[PhoneService] EXECUTE action=" << action << "slotValues=" << slotValues;
    phone_.syncContacts();
    if (action == "PHONE_CALL_BACK") {
        qInfo() << "[PhoneService] TODO: call back recent caller";
        return true;
    }
    if (action == "PHONE_CALL_CONTACT") {
        const QString name = slotValues.value("contact_name").toString().trimmed();
        if (name.isEmpty()) { qWarning() << "[PhoneService] missing contact_name"; return false; }
        const QStringList candidates = contactNameCandidates(name);

        for (const QString& candidate : candidates) {
            if (phone_.hasContact(candidate)) {
                qInfo() << "[PhoneService] resolved contact =" << candidate
                        << "from slot =" << name;
                return phone_.callContact(candidate);
            }
        }

        qWarning() << "[PhoneService] contact not found by HasContact, trying candidates:"
                   << candidates;
        for (const QString& candidate : candidates) {
            if (phone_.callContact(candidate)) {
                qInfo() << "[PhoneService] callContact accepted candidate =" << candidate
                        << "from slot =" << name;
                return true;
            }
        }
        return false;
    }
    if (action == "PHONE_DIAL_NUMBER") {
        const QString number = slotValues.value("phone_number").toString().trimmed();
        if (number.isEmpty()) { qWarning() << "[PhoneService] missing phone_number"; return false; }
        if (!isValidPhoneNumber(number)) {
            qWarning() << "[PhoneService] invalid phone_number:" << number;
            return false;
        }
        return phone_.callNumber(number);
    }
    if (action == "PHONE_HANGUP") return phone_.hangup();
    if (action == "PHONE_ACCEPT_CALL") return phone_.answer();
    if (action == "PHONE_MUTE_MIC") {
        qInfo() << "[PhoneService] TODO: mute phone microphone";
        return true;
    }
    if (action == "PHONE_OPEN_CONTACTS") {
        qInfo() << "[PhoneService] TODO: open contacts UI";
        return true;
    }
    if (action == "PHONE_REDIAL_LAST") {
        qInfo() << "[PhoneService] TODO: redial last number";
        return true;
    }
    if (action == "PHONE_SEARCH_CONTACT") {
        const QString name = slotValues.value("contact_name").toString().trimmed();
        if (name.isEmpty()) { qWarning() << "[PhoneService] missing contact_name"; return false; }
        qInfo() << "[PhoneService] TODO: search contact =" << name;
        return true;
    }
    if (action == "PHONE_SPEAKER_OFF") {
        qInfo() << "[PhoneService] TODO: speaker off";
        return true;
    }
    if (action == "PHONE_SPEAKER_ON") {
        qInfo() << "[PhoneService] TODO: speaker on";
        return true;
    }
    if (action == "PHONE_UNMUTE_MIC") {
        qInfo() << "[PhoneService] TODO: unmute phone microphone";
        return true;
    }
    qWarning() << "[PhoneService] unknown action:" << action;
    return false;
}
