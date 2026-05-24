#include "header_file/fpslogger.h"

#include <QDateTime>
#include <QDir>
#include <QIODevice>
#include <QTextStream>

namespace {

constexpr auto LogPath = "/tmp/myCluster-3d-fps.log";

QString timestamp()
{
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
}

} // namespace

FpsLogger::FpsLogger(QObject *parent)
    : QObject(parent)
    , m_file(QString::fromLatin1(LogPath))
{
}

FpsLogger::~FpsLogger()
{
    stop();
}

QString FpsLogger::logPath() const
{
    return m_file.fileName();
}

bool FpsLogger::ensureOpen()
{
    if (m_file.isOpen())
        return true;

    QDir().mkpath(QStringLiteral("/tmp"));
    return m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
}

void FpsLogger::writeLine(const QString &line)
{
    if (!ensureOpen())
        return;

    QTextStream out(&m_file);
    out << timestamp() << ' ' << line << '\n';
    out.flush();
    m_file.flush();
}

void FpsLogger::start(const QString &label)
{
    if (m_running)
        stop();

    m_label = label;
    m_totalFrames = 0;
    m_bucketFrames = 0;
    m_sessionTimer.restart();
    m_bucketTimer.restart();
    m_running = true;

    writeLine(QStringLiteral("start label=\"%1\" path=\"%2\"")
                      .arg(m_label, m_file.fileName()));
}

void FpsLogger::stop()
{
    if (!m_running)
        return;

    const qint64 elapsedMs = m_sessionTimer.elapsed() > 0 ? m_sessionTimer.elapsed() : 1;
    const double averageFps = (m_totalFrames * 1000.0) / elapsedMs;

    writeLine(QStringLiteral("stop label=\"%1\" total_frames=%2 elapsed_ms=%3 avg_fps=%4")
                      .arg(m_label)
                      .arg(m_totalFrames)
                      .arg(elapsedMs)
                      .arg(averageFps, 0, 'f', 2));

    m_running = false;
}

void FpsLogger::frame()
{
    if (!m_running)
        return;

    ++m_totalFrames;
    ++m_bucketFrames;

    const qint64 bucketMs = m_bucketTimer.elapsed();
    if (bucketMs < 1000)
        return;

    const double fps = (m_bucketFrames * 1000.0) / bucketMs;
    writeLine(QStringLiteral("sample label=\"%1\" fps=%2 frames=%3 bucket_ms=%4 total_frames=%5 elapsed_ms=%6")
                      .arg(m_label)
                      .arg(fps, 0, 'f', 2)
                      .arg(m_bucketFrames)
                      .arg(bucketMs)
                      .arg(m_totalFrames)
                      .arg(m_sessionTimer.elapsed()));

    m_bucketFrames = 0;
    m_bucketTimer.restart();
}

void FpsLogger::mark(const QString &message)
{
    writeLine(QStringLiteral("mark label=\"%1\" %2").arg(m_label, message));
}
