// Verifies that every archive format LogView advertises can be listed and
// extracted through ArchiveCatalog. Runs unchanged on Linux and Windows; the
// fixtures in tests/data are committed so no archive tools are required.
//
// A Windows build without liblzma fails the .xz / .tar.xz / .txz cases here,
// which is exactly the regression this suite guards against.

#include "archive/ArchiveCatalog.hpp"
#include "archive/ArchiveCommon.hpp"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTemporaryDir>
#include <QtTest>

namespace
{

const QByteArray kLogA = "alpha 1\nalpha 2\nalpha 3\n";
const QByteArray kLogB = "bravo 1\nbravo 2\n";

QString fixture(const QString& name)
{
    return QDir(QStringLiteral(LOGVIEW_TEST_DATA_DIR)).filePath(name);
}

QByteArray readFile(const QString& path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

}  // namespace

class ArchiveFormatsTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // ArchiveCommon path helpers.
    void recognisesArchiveFileNames_data();
    void recognisesArchiveFileNames();
    void recognisesNestedLeafNames();
    void splitsNestedEntryPaths_data();
    void splitsNestedEntryPaths();
    void normalizesEntryNames();

    // ArchiveCatalog against real archives.
    void fixturesArePresent_data();
    void fixturesArePresent();
    void listsMultiEntryArchives_data();
    void listsMultiEntryArchives();
    void extractsMultiEntryArchives_data();
    void extractsMultiEntryArchives();
    void extractsSingleStreamFiles_data();
    void extractsSingleStreamFiles();
    void listsNestedZip();
    void extractsFromNestedZip_data();
    void extractsFromNestedZip();
    void reportsErrorForMissingEntry();
    void reportsNoEntriesForMissingFile();

private:
    QTemporaryDir out_;
};

void ArchiveFormatsTest::initTestCase()
{
    QVERIFY2(out_.isValid(), "temporary directory could not be created");
}

// --- ArchiveCommon ----------------------------------------------------------

void ArchiveFormatsTest::recognisesArchiveFileNames_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("expected");

    QTest::newRow("zip") << "logs.zip" << true;
    QTest::newRow("tar.gz") << "logs.tar.gz" << true;
    QTest::newRow("tgz") << "logs.tgz" << true;
    QTest::newRow("tar.xz") << "logs.tar.xz" << true;
    QTest::newRow("txz") << "logs.txz" << true;
    QTest::newRow("log.xz") << "device.log.xz" << true;
    QTest::newRow("log.gz") << "device.log.gz" << true;
    QTest::newRow("upper case") << "LOGS.TAR.XZ" << true;
    QTest::newRow("plain log") << "device.log" << false;
    QTest::newRow("text") << "notes.txt" << false;
    QTest::newRow("xz in the middle") << "xz.log" << false;
}

void ArchiveFormatsTest::recognisesArchiveFileNames()
{
    QFETCH(QString, name);
    QFETCH(bool, expected);
    QCOMPARE(logview_archive::isArchiveFileName(name), expected);
    QCOMPARE(logview_archive::isArchivePath(QStringLiteral("/some/dir/") + name), expected);
}

void ArchiveFormatsTest::recognisesNestedLeafNames()
{
    QVERIFY(logview_archive::isNestedArchiveLeafName(QStringLiteral("dir/inner.log.xz")));
    QVERIFY(logview_archive::isNestedArchiveLeafName(QStringLiteral("inner.tar.gz")));
    QVERIFY(logview_archive::isNestedArchiveLeafName(QStringLiteral("\\dir\\inner.zip")));
    QVERIFY(!logview_archive::isNestedArchiveLeafName(QStringLiteral("dir/inner.log")));
}

void ArchiveFormatsTest::splitsNestedEntryPaths_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<bool>("nested");
    QTest::addColumn<QString>("outer");
    QTest::addColumn<QString>("inner");

    QTest::newRow("plain entry") << "logs/a.log" << false << QString() << QString();
    QTest::newRow("zip member") << "inner.zip/a.log" << true << "inner.zip" << "a.log";
    QTest::newRow("tar.xz member") << "dir/in.tar.xz/logs/a.log" << true
                                   << "dir/in.tar.xz" << "logs/a.log";
    QTest::newRow("txz member") << "in.txz/a.log" << true << "in.txz" << "a.log";
    QTest::newRow("outermost wins") << "o.zip/i.tar.gz/a.log" << true
                                    << "o.zip" << "i.tar.gz/a.log";
    QTest::newRow("backslashes") << "o.zip\\a.log" << true << "o.zip" << "a.log";
}

void ArchiveFormatsTest::splitsNestedEntryPaths()
{
    QFETCH(QString, path);
    QFETCH(bool, nested);
    QFETCH(QString, outer);
    QFETCH(QString, inner);

    const auto split = logview_archive::splitNestedEntryPath(path);
    QCOMPARE(split.has_value(), nested);
    if (nested)
    {
        QCOMPARE(split->outer_entry, outer);
        QCOMPARE(split->inner_path, inner);
    }
}

void ArchiveFormatsTest::normalizesEntryNames()
{
    QCOMPARE(logview_archive::normalizeEntryName(QStringLiteral("\\a\\b.log")),
             QStringLiteral("a/b.log"));
    QCOMPARE(logview_archive::normalizeEntryName(QStringLiteral("//a/b.log")),
             QStringLiteral("a/b.log"));
}

// --- ArchiveCatalog ---------------------------------------------------------

void ArchiveFormatsTest::fixturesArePresent_data()
{
    QTest::addColumn<QString>("file");
    for (const char* name : {"multi.zip", "multi.tar.gz", "multi.tgz", "multi.tar.xz",
                             "multi.txz", "single.log.gz", "single.log.xz", "nested.zip"})
        QTest::newRow(name) << QString::fromLatin1(name);
}

void ArchiveFormatsTest::fixturesArePresent()
{
    QFETCH(QString, file);
    QVERIFY2(QFileInfo::exists(fixture(file)), qPrintable(fixture(file)));
}

void ArchiveFormatsTest::listsMultiEntryArchives_data()
{
    QTest::addColumn<QString>("file");
    QTest::newRow("zip") << "multi.zip";
    QTest::newRow("tar.gz") << "multi.tar.gz";
    QTest::newRow("tgz") << "multi.tgz";
    QTest::newRow("tar.xz") << "multi.tar.xz";
    QTest::newRow("txz") << "multi.txz";
}

void ArchiveFormatsTest::listsMultiEntryArchives()
{
    QFETCH(QString, file);
    const ArchiveCatalogResult result = ArchiveCatalog::build(fixture(file));
    QVERIFY(!result.truncated);
    QCOMPARE(result.entry_paths,
             (QStringList{QStringLiteral("logs/a.log"), QStringLiteral("logs/b.log")}));
}

void ArchiveFormatsTest::extractsMultiEntryArchives_data()
{
    listsMultiEntryArchives_data();
}

void ArchiveFormatsTest::extractsMultiEntryArchives()
{
    QFETCH(QString, file);
    const QString dest_a = out_.filePath(QStringLiteral("multi_a.log"));
    const QString dest_b = out_.filePath(QStringLiteral("multi_b.log"));
    QString error;

    QVERIFY2(ArchiveCatalog::extractNestedEntry(fixture(file), QStringLiteral("logs/a.log"),
                                                dest_a, &error),
             qPrintable(error));
    QVERIFY2(ArchiveCatalog::extractNestedEntry(fixture(file), QStringLiteral("logs/b.log"),
                                                dest_b, &error),
             qPrintable(error));
    QCOMPARE(readFile(dest_a), kLogA);
    QCOMPARE(readFile(dest_b), kLogB);
}

void ArchiveFormatsTest::extractsSingleStreamFiles_data()
{
    QTest::addColumn<QString>("file");
    QTest::newRow("log.gz") << "single.log.gz";
    QTest::newRow("log.xz") << "single.log.xz";
}

void ArchiveFormatsTest::extractsSingleStreamFiles()
{
    QFETCH(QString, file);

    // A single compressed stream has exactly one payload entry.
    const ArchiveCatalogResult result = ArchiveCatalog::build(fixture(file));
    QCOMPARE(result.entry_paths.size(), 1);

    const QString dest = out_.filePath(QStringLiteral("single_") + file);
    QString error;
    QVERIFY2(ArchiveCatalog::extractNestedEntry(fixture(file), result.entry_paths.first(),
                                                dest, &error),
             qPrintable(error));
    QCOMPARE(readFile(dest), kLogA);
}

void ArchiveFormatsTest::listsNestedZip()
{
    const ArchiveCatalogResult result = ArchiveCatalog::build(fixture(QStringLiteral("nested.zip")));
    QVERIFY(!result.truncated);

    // The single-stream members stay as the compressed file to open; the
    // tar.xz is expanded into its members.
    const QStringList expected{
        QStringLiteral("inner.log.gz"),
        QStringLiteral("inner.log.xz"),
        QStringLiteral("inner.tar.xz/logs/a.log"),
        QStringLiteral("inner.tar.xz/logs/b.log"),
        QStringLiteral("top.log"),
    };
    QCOMPARE(result.entry_paths, expected);
}

void ArchiveFormatsTest::extractsFromNestedZip_data()
{
    QTest::addColumn<QString>("entry");
    QTest::addColumn<QByteArray>("expected");

    QTest::newRow("plain member") << "top.log" << kLogB;
    QTest::newRow("log.gz member") << "inner.log.gz" << kLogA;
    QTest::newRow("log.xz member") << "inner.log.xz" << kLogA;
    QTest::newRow("tar.xz member a") << "inner.tar.xz/logs/a.log" << kLogA;
    QTest::newRow("tar.xz member b") << "inner.tar.xz/logs/b.log" << kLogB;
}

void ArchiveFormatsTest::extractsFromNestedZip()
{
    QFETCH(QString, entry);
    QFETCH(QByteArray, expected);

    const QString dest = out_.filePath(QStringLiteral("nested_out.log"));
    QString error;
    QVERIFY2(ArchiveCatalog::extractNestedEntry(fixture(QStringLiteral("nested.zip")), entry,
                                                dest, &error),
             qPrintable(error));
    QCOMPARE(readFile(dest), expected);
}

void ArchiveFormatsTest::reportsErrorForMissingEntry()
{
    QString error;
    const QString dest = out_.filePath(QStringLiteral("missing.log"));
    QVERIFY(!ArchiveCatalog::extractNestedEntry(fixture(QStringLiteral("multi.tar.xz")),
                                                QStringLiteral("logs/nope.log"), dest, &error));
    QVERIFY(!error.isEmpty());
    QVERIFY(!QFileInfo::exists(dest));
}

void ArchiveFormatsTest::reportsNoEntriesForMissingFile()
{
    const ArchiveCatalogResult result = ArchiveCatalog::build(fixture(QStringLiteral("does-not-exist.zip")));
    QVERIFY(result.entry_paths.isEmpty());

    QString error;
    QVERIFY(!ArchiveCatalog::extractNestedEntry(fixture(QStringLiteral("does-not-exist.zip")),
                                                QStringLiteral("a.log"),
                                                out_.filePath(QStringLiteral("x.log")), &error));
    QVERIFY(!error.isEmpty());
}

QTEST_GUILESS_MAIN(ArchiveFormatsTest)
#include "ArchiveFormatsTest.moc"
