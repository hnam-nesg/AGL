#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QtCore/QObject>

class  QTranslator;

class Translator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
public:
    explicit Translator(QObject *parent = nullptr);

    QString language() const;

    Q_INVOKABLE QString translate(const QString &string, const QString &language) const;
public slots:
    void setLanguage(const QString &language);

signals:
    void languageChanged(const QString &language);

private slots:
    void setTranslator(const QString &language);

private:
    QString m_language;
    QTranslator *m_translator;
};

#endif // TRANSLATOR_H
