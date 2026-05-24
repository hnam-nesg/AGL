#include "audiocontroller.h"

#include <QProcess>
#include <QRegularExpression>
#include <QtGlobal>
#include <QDebug>
#include <cmath>

AudioController::AudioController(QObject *parent)
    : QObject(parent)
{
}

double AudioController::volume() const
{
    return m_volume;
}

bool AudioController::muted() const
{
    return m_muted;
}

QString AudioController::appNameHint() const
{
    return m_appNameHint;
}

QString AudioController::deviceAddress() const
{
    return m_deviceAddress;
}

void AudioController::setAppNameHint(const QString &name)
{
    if (m_appNameHint == name)
        return;

    m_appNameHint = name;
    emit appNameHintChanged();

    refreshTarget();
}

void AudioController::setDeviceAddress(const QString &address)
{
    QString normalized = address.trimmed().toUpper();
    if (normalized == m_deviceAddress)
        return;

    m_deviceAddress = normalized;
    emit deviceAddressChanged();
    refreshTarget();
}

QString AudioController::runCommand(const QString &program, const QStringList &args) const
{
    QProcess process;
    process.start(program, args);
    process.waitForFinished(3000);

    QString out = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString err = QString::fromLocal8Bit(process.readAllStandardError());

    if (!err.trimmed().isEmpty())
        qWarning() << program << args << err;

    return out;
}

QString AudioController::findBluetoothNodeIdByAddress() const
{
    if (m_deviceAddress.isEmpty())
        return QString();

    const QString status = runCommand("wpctl", {"status"});
    if (status.isEmpty())
        return QString();

    const QString needle = m_deviceAddress;
    const QString nodeNeedle = needle;
    const QStringList lines = status.split('\n');
    QRegularExpression streamRegex("(\\d+)\\.\\s+(bluez_[^\\s]+)");

    for (const QString &line : lines) {
        const QString lower = line.toLower();
        if (!lower.contains("bluez_"))
            continue;
        if (!lower.contains(nodeNeedle.toLower().replace(':', '_')))
            continue;

        QRegularExpressionMatch match = streamRegex.match(line);
        if (match.hasMatch())
            return match.captured(1);
    }

    return QString();
}

QString AudioController::findStreamIdByHint() const
{
    const QString status = runCommand("wpctl", {"status"});
    if (status.isEmpty())
        return QString();

    const QStringList lines = status.split('\n');

    bool inStreams = false;
    QRegularExpression streamTopRegex("^\\s{6,}(\\d+)\\.\\s+(.+)$");
    QRegularExpression childRegex("^\\s{10,}(\\d+)\\.\\s+(.+)$");

    for (const QString &line : lines) {
        if (line.contains("└─ Streams:")) {
            inStreams = true;
            continue;
        }

        if (!inStreams)
            continue;

        if (line.startsWith("Video") || line.startsWith("Settings"))
            break;

        // Bỏ qua dòng con như 176, 177
        if (childRegex.match(line).hasMatch())
            continue;

        QRegularExpressionMatch match = streamTopRegex.match(line);
        if (!match.hasMatch())
            continue;

        const QString id = match.captured(1);
        const QString name = match.captured(2).trimmed();

        if (name.toLower().contains(m_appNameHint.toLower()) || inspectMatches(id))
            return id;
    }

    return QString();
}

bool AudioController::inspectMatches(const QString &id) const
{
    const QString inspect = runCommand("wpctl", {"inspect", id});
    if (inspect.isEmpty())
        return false;

    const QString haystack = inspect.toLower();
    const QString hint = m_appNameHint.toLower();

    // Bạn có thể thêm điều kiện nếu app của bạn hiện ra bằng tên khác
    if (haystack.contains("application.name") && haystack.contains(hint))
        return true;

    if (haystack.contains("node.name") && haystack.contains(hint))
        return true;

    if (haystack.contains("media.name") && haystack.contains(hint))
        return true;

    return false;
}

QString AudioController::resolveTargetId()
{
    QString id;
    if (!m_deviceAddress.isEmpty()) {
        id = findBluetoothNodeIdByAddress();
    } else {
        id = findStreamIdByHint();
    }

    if (id.isEmpty()) {
        if (!m_deviceAddress.isEmpty()) {
            qWarning() << "No Bluetooth audio node matched address" << m_deviceAddress
                       << "- volume changes will be ignored until it appears";
        } else {
            qWarning() << "No audio stream matched hint" << m_appNameHint
                       << "- volume changes will be ignored until it appears";
        }
    } else {
        qDebug() << "Resolved audio target =" << id;
    }

    if (m_targetId != id) {
        m_targetId = id;
        emit targetChanged();
    }
    return m_targetId;
}

void AudioController::refreshTarget()
{
    resolveTargetId();
}

void AudioController::setVolume(double value)
{
    value = qBound(0.0, value, 1.0);
    if (!qFuzzyCompare(m_volume + 1.0, value + 1.0)) {
        m_volume = value;
        emit volumeChanged();
    }

    if (m_targetId.isEmpty())
        resolveTargetId();

    if (m_targetId.isEmpty()) {
        qWarning() << "set-volume skipped: no stream matched hint" << m_appNameHint;
        return;
    }

    const QString volStr = QString::number(value, 'f', 2);

    // nếu stream id cũ mất rồi thì tìm lại
    int rc = QProcess::execute("wpctl",
                               QStringList() << "set-volume"
                                             << m_targetId
                                             << volStr);

    if (rc != 0) {
        qWarning() << "set-volume failed for target" << m_targetId
                   << ", retry resolving target";
        resolveTargetId();

        if (m_targetId.isEmpty())
            return;

        QProcess::execute("wpctl",
                          QStringList() << "set-volume"
                                        << m_targetId
                                        << volStr);
    }
}

void AudioController::setVolumeMute(bool mute)
{
    if (m_muted != mute) {
        m_muted = mute;
        emit mutedChanged();
    }

    if (m_targetId.isEmpty())
        resolveTargetId();

    if (m_targetId.isEmpty()) {
        qWarning() << "set-mute skipped: no stream matched hint" << m_appNameHint;
        return;
    }

    int rc = QProcess::execute("wpctl",
                               QStringList() << "set-mute"
                                             << m_targetId
                                             << (mute ? "1" : "0"));

    if (rc != 0) {
        qWarning() << "set-mute failed for target" << m_targetId
                   << ", retry resolving target";
        resolveTargetId();

        if (m_targetId.isEmpty())
            return;

        QProcess::execute("wpctl",
                          QStringList() << "set-mute"
                                        << m_targetId
                                        << (mute ? "1" : "0"));
    }
}
