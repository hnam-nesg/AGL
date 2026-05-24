#include "translator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtCore/QDir>
#include <QtCore/QDebug>

Translator::Translator(QObject *parent)
    : QObject(parent)
    , m_language(QStringLiteral("C"))
    , m_translator(nullptr)
{
}

QString Translator::translate(const QString &string, const QString &language) const
{
    Q_UNUSED(language)
    return string;
}

QString Translator::language() const
{
    return m_language;
}

void Translator::setLanguage(const QString &language)
{
    if (m_language == language) return;
    m_language = language;
    setTranslator(language);
    emit languageChanged(language);
}

void Translator::setTranslator(const QString &language)
{
    if (m_translator) {
        QCoreApplication::removeTranslator(m_translator);
    } else {
        m_translator = new QTranslator(this);
    }
    QLocale locale(language);
    QString fileName = QCoreApplication::instance()->applicationName().toLower();
    if (m_translator->load(locale, fileName, QStringLiteral("_"), QStringLiteral("/usr/share/%1").arg(fileName))) {
        QCoreApplication::installTranslator(m_translator);
    } else {
        delete m_translator;
        m_translator = nullptr;
    }
}

