#pragma once

#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>

// Custom, key-based localization.
//
// UI strings are referenced by semantic keys (e.g. "menu.file") and resolved at
// runtime from plain "key=value" catalogs in the ./lang directory (en.lang,
// pl.lang, ...). English (en.lang) is always loaded and used as the fallback.
//
// This intentionally does NOT use QTranslator/tr(): switching language reloads
// the active catalog and posts a QEvent::LanguageChange to every top-level
// widget so widgets that override changeEvent() can retranslate live.
class Translator
{
public:
    static Translator& instance();

    static constexpr auto kSystem = "system";
    static constexpr auto kEnglish = "en";

    // "system" plus every <code>.lang discovered in the lang directory.
    static QStringList availableLanguageCodes();

    // Human-readable name for a code, read from the catalog's "language.name"
    // key ("system" is localized via "settings.language.system").
    static QString displayName(const QString& code);

    // Loads the catalog for the given language code and notifies the UI.
    // "system" resolves to the OS locale.
    void setLanguage(const QString& code);

    QString language() const { return language_; }

    // Looks up a key in the active catalog, falling back to English and finally
    // to the key itself (so missing translations are obvious in the UI). Missing
    // keys are logged once as a warning on the console.
    QString text(const QString& key) const;

private:
    Translator() = default;
    Translator(const Translator&) = delete;
    Translator& operator=(const Translator&) = delete;

    void ensureEnglishLoaded();
    static void notifyLanguageChanged();

    static QString languageDirectory();
    static QString resolveCode(const QString& code);
    static QHash<QString, QString> loadCatalog(const QString& code);

    QString language_;
    QHash<QString, QString> active_;
    QHash<QString, QString> english_;
    mutable QSet<QString> warned_keys_;
};

// Convenience lookup used throughout the UI: Lang::tr("menu.file").
namespace Lang
{
QString tr(const QString& key);
}
