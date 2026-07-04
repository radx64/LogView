#include "Translator.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QTextStream>
#include <QWidget>

namespace
{
// Turns the escape sequences used in .lang files back into their characters.
// Only "\n" is special; everything else (including lone backslashes found in
// regex examples) is left untouched.
QString unescape(const QString& in)
{
    QString out;
    out.reserve(in.size());
    for (int i = 0; i < in.size(); ++i)
    {
        if (in[i] == QLatin1Char('\\') && i + 1 < in.size() &&
            in[i + 1] == QLatin1Char('n'))
        {
            out += QLatin1Char('\n');
            ++i;
        }
        else
        {
            out += in[i];
        }
    }
    return out;
}

constexpr auto kLanguageNameKey = "language.name";
} // namespace

Translator& Translator::instance()
{
    static Translator translator;
    return translator;
}

QString Translator::languageDirectory()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        appDir + QStringLiteral("/lang"),
        appDir + QStringLiteral("/../lang"),
        QDir::currentPath() + QStringLiteral("/lang"),
    };
    for (const QString& candidate : candidates)
    {
        if (QDir(candidate).exists())
            return QDir(candidate).absolutePath();
    }
    return appDir + QStringLiteral("/lang");
}

QHash<QString, QString> Translator::loadCatalog(const QString& code)
{
    QHash<QString, QString> catalog;

    QFile file(languageDirectory() + QStringLiteral("/") + code +
               QStringLiteral(".lang"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return catalog;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    while (!stream.atEnd())
    {
        const QString line = stream.readLine();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
            continue;

        const int separator = line.indexOf(QLatin1Char('='));
        if (separator < 0)
            continue;

        const QString key = line.left(separator).trimmed();
        const QString value = unescape(line.mid(separator + 1));
        if (!key.isEmpty())
            catalog.insert(key, value);
    }
    return catalog;
}

QStringList Translator::availableLanguageCodes()
{
    QStringList codes{kSystem};

    const QDir dir(languageDirectory());
    const QStringList files =
        dir.entryList({QStringLiteral("*.lang")}, QDir::Files, QDir::Name);
    for (const QString& file : files)
    {
        const QString code = QFileInfo(file).completeBaseName();
        if (!code.isEmpty() && !codes.contains(code))
            codes.append(code);
    }
    return codes;
}

QString Translator::displayName(const QString& code)
{
    if (code == kSystem || code.isEmpty())
        return instance().text(QStringLiteral("settings.language.system"));

    const QHash<QString, QString> catalog = loadCatalog(code);
    const QString name = catalog.value(QLatin1String(kLanguageNameKey));
    return name.isEmpty() ? code : name;
}

QString Translator::resolveCode(const QString& code)
{
    if (code == kSystem || code.isEmpty())
        return QLocale::system().name().startsWith(QLatin1String("pl"))
                   ? QStringLiteral("pl")
                   : QString::fromLatin1(kEnglish);
    return code;
}

void Translator::ensureEnglishLoaded()
{
    if (english_.isEmpty())
        english_ = loadCatalog(kEnglish);
}

void Translator::notifyLanguageChanged()
{
    if (qApp == nullptr)
        return;
    const QList<QWidget*> widgets = QApplication::topLevelWidgets();
    for (QWidget* widget : widgets)
        QCoreApplication::postEvent(widget, new QEvent(QEvent::LanguageChange));
}

void Translator::setLanguage(const QString& code)
{
    ensureEnglishLoaded();

    language_ = code.isEmpty() ? QString::fromLatin1(kSystem) : code;
    const QString effective = resolveCode(language_);

    active_ = (effective == QLatin1String(kEnglish)) ? english_
                                                     : loadCatalog(effective);

    notifyLanguageChanged();
}

QString Translator::text(const QString& key) const
{
    auto it = active_.constFind(key);
    if (it != active_.constEnd())
        return it.value();

    it = english_.constFind(key);
    if (it != english_.constEnd())
        return it.value();

    return key;
}

QString Lang::tr(const QString& key)
{
    return Translator::instance().text(key);
}
