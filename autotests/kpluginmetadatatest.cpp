/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QPluginLoader>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTest>

#include "kcoreaddons_debug.h"
#include <kaboutdata.h>
#include <kpluginloader.h>
#include <kpluginmetadata.h>

#include <QLocale>
#include <QLoggingCategory>

namespace QTest
{
template<>
inline char *toString(const QJsonValue &val)
{
    // simply reuse the QDebug representation
    QString result;
    QDebug(&result) << val;
    return QTest::toString(result);
}
}

class LibraryPathRestorer
{
public:
    explicit LibraryPathRestorer(const QStringList &paths)
        : mPaths(paths)
    {
    }
    ~LibraryPathRestorer()
    {
        QCoreApplication::setLibraryPaths(mPaths);
    }

private:
    QStringList mPaths;
};

class KPluginMetaDataTest : public QObject
{
    Q_OBJECT
    bool m_canMessage = false;

    void doMessagesWorkInternal()
    {
    }

    Q_REQUIRED_RESULT bool doMessagesWork()
    {
        // Q_SKIP returns, but since this is called in multiple tests we want to return a bool so the caller can
        // return easily.
        auto internalCheck = [this] {
            // Make sure output is well formed AND generated. To that end we cannot run this test when any of the
            // overriding environment variables are set.
            // https://bugs.kde.org/show_bug.cgi?id=387006
            if (qEnvironmentVariableIsSet("QT_MESSAGE_PATTERN")) {
                QSKIP("QT_MESSAGE_PATTERN prevents warning expectations from matching");
            }
            if (qEnvironmentVariableIsSet("QT_LOGGING_RULES")) {
                QSKIP("QT_LOGGING_RULES prevents warning expectations from matching");
            }
            if (qEnvironmentVariableIsSet("QT_LOGGING_CONF")) {
                QSKIP("QT_LOGGING_CONF prevents warning expectations from matching");
            }
            m_canMessage = true;
            // Ensure all frameworks output is enabled so the expectations can match.
            // qtlogging.ini may have disabled it but we can fix that because setFilterRules overrides the ini files.
            QLoggingCategory::setFilterRules(QStringLiteral("kf.*=true"));
        };
        internalCheck();
        return m_canMessage;
    }
private Q_SLOTS:

    void testFromPluginLoader()
    {
        QString location;
        location = QPluginLoader(QStringLiteral("jsonplugin")).fileName();
        QVERIFY2(!location.isEmpty(), "Could not find jsonplugin");

        // now that this file is translated we need to read it instead of hardcoding the contents here
        QString jsonLocation = QFINDTESTDATA("jsonplugin.json");
        QVERIFY2(!jsonLocation.isEmpty(), "Could not find jsonplugin.json");
        QFile jsonFile(jsonLocation);
        QVERIFY(jsonFile.open(QFile::ReadOnly));
        QJsonParseError e;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), &e);
        QCOMPARE(e.error, QJsonParseError::NoError);

        location = QFileInfo(location).absoluteFilePath();

        KPluginMetaData fromQPluginLoader(QPluginLoader(QStringLiteral("jsonplugin")));
        KPluginMetaData fromFullPath(location);
        KPluginMetaData fromRelativePath(QStringLiteral("jsonplugin"));
        KPluginMetaData fromRawData(jsonDoc.object(), location);

        auto description = QStringLiteral("This is a plugin");

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 86)
        KPluginMetaData fromKPluginLoader(KPluginLoader(QStringLiteral("jsonplugin")));
        QVERIFY(fromKPluginLoader.isValid());
        QCOMPARE(fromKPluginLoader.description(), description);
        QCOMPARE(fromKPluginLoader, fromKPluginLoader);
        QCOMPARE(fromQPluginLoader, fromKPluginLoader);
        QCOMPARE(fromKPluginLoader, fromQPluginLoader);
        QCOMPARE(fromKPluginLoader, fromFullPath);
        QCOMPARE(fromKPluginLoader, fromRawData);
        QCOMPARE(fromFullPath, fromKPluginLoader);
        QCOMPARE(fromRawData, fromKPluginLoader);
        QVERIFY(!KPluginMetaData(KPluginLoader(QStringLiteral("doesnotexist"))).isValid());
#endif

        QVERIFY(fromQPluginLoader.isValid());
        QCOMPARE(fromQPluginLoader.description(), description);
        QVERIFY(fromFullPath.isValid());
        QCOMPARE(fromFullPath.description(), description);
        QVERIFY(fromRelativePath.isValid());
        QCOMPARE(fromRelativePath.description(), description);
        QVERIFY(fromRawData.isValid());
        QCOMPARE(fromRawData.description(), description);

        // check operator==
        QCOMPARE(fromRawData, fromRawData);
        QCOMPARE(fromQPluginLoader, fromQPluginLoader);
        QCOMPARE(fromFullPath, fromFullPath);

        QCOMPARE(fromQPluginLoader, fromFullPath);
        QCOMPARE(fromQPluginLoader, fromRawData);

        QCOMPARE(fromFullPath, fromQPluginLoader);
        QCOMPARE(fromFullPath, fromRawData);

        QCOMPARE(fromRawData, fromQPluginLoader);
        QCOMPARE(fromRawData, fromFullPath);

        QVERIFY(!KPluginMetaData(QPluginLoader(QStringLiteral("doesnotexist"))).isValid());
        QVERIFY(!KPluginMetaData(QJsonObject(), QString()).isValid());
    }

    void testAllKeys()
    {
        QJsonParseError e;
        QJsonObject jo = QJsonDocument::fromJson(
                             "{\n"
                             " \"KPlugin\": {\n"
                             " \"Name\": \"Date and Time\",\n"
                             " \"Description\": \"Date and time by timezone\",\n"
                             " \"Icon\": \"preferences-system-time\",\n"
                             " \"Authors\": { \"Name\": \"Aaron Seigo\", \"Email\": \"aseigo@kde.org\" },\n"
                             " \"Translators\": { \"Name\": \"No One\", \"Email\": \"no.one@kde.org\" },\n"
                             " \"OtherContributors\": { \"Name\": \"No One\", \"Email\": \"no.one@kde.org\" },\n"
                             " \"Category\": \"Date and Time\",\n"
                             " \"Dependencies\": [ \"foo\", \"bar\"],\n"
                             " \"EnabledByDefault\": \"true\",\n"
                             " \"ExtraInformation\": \"Something else\",\n"
                             " \"License\": \"LGPL\",\n"
                             " \"Copyright\": \"(c) Alex Richardson 2015\",\n"
                             " \"Id\": \"time\",\n"
                             " \"Version\": \"1.0\",\n"
                             " \"Website\": \"https://plasma.kde.org/\",\n"
                             " \"MimeTypes\": [ \"image/png\" ],\n"
                             " \"ServiceTypes\": [\"Plasma/DataEngine\"]\n"
                             " }\n}\n",
                             &e)
                             .object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        KPluginMetaData m(jo, QString());
        QVERIFY(m.isValid());
        QCOMPARE(m.pluginId(), QStringLiteral("time"));
        QCOMPARE(m.name(), QStringLiteral("Date and Time"));
        QCOMPARE(m.description(), QStringLiteral("Date and time by timezone"));
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 87)
        QCOMPARE(m.extraInformation(), QStringLiteral("Something else"));
#endif
        QCOMPARE(m.iconName(), QStringLiteral("preferences-system-time"));
        QCOMPARE(m.category(), QStringLiteral("Date and Time"));
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 79)
        QCOMPARE(m.dependencies(), QStringList() << QStringLiteral("foo") << QStringLiteral("bar"));
#endif
        QCOMPARE(m.authors().size(), 1);
        QCOMPARE(m.authors().constFirst().name(), QStringLiteral("Aaron Seigo"));
        QCOMPARE(m.authors().constFirst().emailAddress(), QStringLiteral("aseigo@kde.org"));
        QCOMPARE(m.translators().size(), 1);
        QCOMPARE(m.translators().constFirst().name(), QStringLiteral("No One"));
        QCOMPARE(m.translators().constFirst().emailAddress(), QStringLiteral("no.one@kde.org"));
        QCOMPARE(m.otherContributors().size(), 1);
        QCOMPARE(m.otherContributors().constFirst().name(), QStringLiteral("No One"));
        QCOMPARE(m.otherContributors().constFirst().emailAddress(), QStringLiteral("no.one@kde.org"));
        QVERIFY(m.isEnabledByDefault());
        QCOMPARE(m.license(), QStringLiteral("LGPL"));
        QCOMPARE(m.copyrightText(), QStringLiteral("(c) Alex Richardson 2015"));
        QCOMPARE(m.version(), QStringLiteral("1.0"));
        QCOMPARE(m.website(), QStringLiteral("https://plasma.kde.org/"));
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 89)
        QCOMPARE(m.serviceTypes(), QStringList() << QStringLiteral("Plasma/DataEngine"));
#endif
        QCOMPARE(m.mimeTypes(), QStringList() << QStringLiteral("image/png"));
    }

    void testTranslations()
    {
        QJsonParseError e;
        QJsonObject jo = QJsonDocument::fromJson(
                             "{ \"KPlugin\": {\n"
                             "\"Name\": \"Name\",\n"
                             "\"Name[de]\": \"Name (de)\",\n"
                             "\"Name[de_DE]\": \"Name (de_DE)\",\n"
                             "\"Description\": \"Description\",\n"
                             "\"Description[de]\": \"Beschreibung (de)\",\n"
                             "\"Description[de_DE]\": \"Beschreibung (de_DE)\"\n"
                             "}\n}",
                             &e)
                             .object();
        KPluginMetaData m(jo, QString());
        QLocale::setDefault(QLocale::c());
        QCOMPARE(m.name(), QStringLiteral("Name"));
        QCOMPARE(m.description(), QStringLiteral("Description"));

        QLocale::setDefault(QLocale(QStringLiteral("de_DE")));
        QCOMPARE(m.name(), QStringLiteral("Name (de_DE)"));
        QCOMPARE(m.description(), QStringLiteral("Beschreibung (de_DE)"));

        QLocale::setDefault(QLocale(QStringLiteral("de_CH")));
        QCOMPARE(m.name(), QStringLiteral("Name (de)"));
        QCOMPARE(m.description(), QStringLiteral("Beschreibung (de)"));

        QLocale::setDefault(QLocale(QStringLiteral("fr_FR")));
        QCOMPARE(m.name(), QStringLiteral("Name"));
        QCOMPARE(m.description(), QStringLiteral("Description"));
    }

    void testReadStringList()
    {
        if (!doMessagesWork()) {
            return;
        }
        QJsonParseError e;
        QJsonObject jo = QJsonDocument::fromJson(
                             "{\n"
                             "\"String\": \"foo\",\n"
                             "\"OneArrayEntry\": [ \"foo\" ],\n"
                             "\"Bool\": true,\n" // make sure booleans are accepted
                             "\"QuotedBool\": \"true\",\n" // make sure booleans are accepted
                             "\"Number\": 12345,\n" // number should also work
                             "\"QuotedNumber\": \"12345\",\n" // number should also work
                             "\"EmptyArray\": [],\n"
                             "\"NumberArray\": [1, 2, 3],\n"
                             "\"BoolArray\": [true, false, true],\n"
                             "\"StringArray\": [\"foo\", \"bar\"],\n"
                             "\"Null\": null,\n" // should return empty list
                             "\"QuotedNull\": \"null\",\n" // this is okay, it is a string
                             "\"ArrayWithNull\": [ \"foo\", null, \"bar\"],\n" // TODO: null is converted to empty string, is this okay?
                             "\"Object\": { \"foo\": \"bar\" }\n" // should return empty list
                             "}",
                             &e)
                             .object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Expected JSON property ")));
        KPluginMetaData data(jo, QStringLiteral("test"));
        QCOMPARE(data.value(QStringLiteral("String"), QStringList()), QStringList(QStringLiteral("foo")));
        QCOMPARE(data.value(QStringLiteral("OneArrayEntry"), QStringList()), QStringList(QStringLiteral("foo")));
        QCOMPARE(data.value(QStringLiteral("Bool"), QStringList()), QStringList(QStringLiteral("true")));
        QCOMPARE(data.value(QStringLiteral("QuotedBool"), QStringList()), QStringList(QStringLiteral("true")));
        QCOMPARE(data.value(QStringLiteral("Number"), QStringList()), QStringList(QStringLiteral("12345")));
        QCOMPARE(data.value(QStringLiteral("QuotedNumber"), QStringList()), QStringList(QStringLiteral("12345")));
        QCOMPARE(data.value(QStringLiteral("EmptyArray"), QStringList()), QStringList());
        QCOMPARE(data.value(QStringLiteral("NumberArray"), QStringList()), QStringList() << QStringLiteral("1") << QStringLiteral("2") << QStringLiteral("3"));
        QCOMPARE(data.value(QStringLiteral("BoolArray"), QStringList()),
                 QStringList() << QStringLiteral("true") << QStringLiteral("false") << QStringLiteral("true"));
        QCOMPARE(data.value(QStringLiteral("StringArray"), QStringList()), QStringList() << QStringLiteral("foo") << QStringLiteral("bar"));
        QCOMPARE(data.value(QStringLiteral("Null"), QStringList()), QStringList());
        QCOMPARE(data.value(QStringLiteral("QuotedNull"), QStringList()), QStringList(QStringLiteral("null")));
        QCOMPARE(data.value(QStringLiteral("ArrayWithNull"), QStringList()), QStringList() << QStringLiteral("foo") << QString() << QStringLiteral("bar"));
        QCOMPARE(data.value(QStringLiteral("Object"), QStringList()), QStringList());
    }

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 91)
    void testFromDesktopFile()
    {
        const QString dfile = QFINDTESTDATA("data/fakeplugin.desktop");
        KPluginMetaData md = KPluginMetaData::fromDesktopFile(dfile);
        QVERIFY(md.isValid());
        QCOMPARE(md.pluginId(), QStringLiteral("fakeplugin"));
        QCOMPARE(md.fileName(), QStringLiteral("fakeplugin"));
        QCOMPARE(md.metaDataFileName(), dfile);
        QCOMPARE(md.iconName(), QStringLiteral("preferences-system-time"));
        QCOMPARE(md.license(), QStringLiteral("LGPL"));
        QCOMPARE(md.website(), QStringLiteral("https://kde.org/"));
        QCOMPARE(md.category(), QStringLiteral("Examples"));
        QCOMPARE(md.version(), QStringLiteral("1.0"));
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 79)
        QCOMPARE(md.dependencies(), QStringList());
#endif
        QCOMPARE(md.isHidden(), false);
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 89)
        QCOMPARE(md.serviceTypes(), QStringList(QStringLiteral("KService/NSA")));
#endif
        QCOMPARE(md.mimeTypes(), QStringList() << QStringLiteral("image/png") << QStringLiteral("application/pdf"));

        auto kp = md.rawData()[QStringLiteral("KPlugin")].toObject();
        QStringList formFactors = kp.value(QStringLiteral("FormFactors")).toVariant().toStringList();
        QCOMPARE(formFactors, QStringList() << QStringLiteral("mediacenter") << QStringLiteral("desktop"));
        QCOMPARE(md.formFactors(), QStringList() << QStringLiteral("mediacenter") << QStringLiteral("desktop"));

        const QString dfilehidden = QFINDTESTDATA("data/hiddenplugin.desktop");
        KPluginMetaData mdhidden = KPluginMetaData::fromDesktopFile(dfilehidden);
        QVERIFY(mdhidden.isValid());
        QCOMPARE(mdhidden.isHidden(), true);
    }

    void twoStepsParseTest()
    {
        QStandardPaths::setTestModeEnabled(true);
        const QString dfile = QFINDTESTDATA("data/twostepsparsetest.desktop");
        const QString typesPath = QFINDTESTDATA("data/servicetypes/example-servicetype.desktop");
        KPluginMetaData md = KPluginMetaData::fromDesktopFile(dfile, QStringList() << typesPath);
        QVERIFY(md.isValid());
        QStringList list = md.value(QStringLiteral("X-Test-List"), QStringList());
        QCOMPARE(list, QStringList({QStringLiteral("first"), QStringLiteral("second")}));
    }

    void testServiceTypes_data()
    {
        const QString kdevServiceTypePath = QFINDTESTDATA("data/servicetypes/fake-kdevelopplugin.desktop");
        const QString invalidServiceTypePath = QFINDTESTDATA("data/servicetypes/invalid-servicetype.desktop");
        const QString exampleServiceTypePath = QFINDTESTDATA("data/servicetypes/example-servicetype.desktop");
        QVERIFY(!kdevServiceTypePath.isEmpty());
        QVERIFY(!invalidServiceTypePath.isEmpty());
        QVERIFY(!exampleServiceTypePath.isEmpty());
    }

    void testServiceType()
    {
        if (!doMessagesWork()) {
            return;
        }
        const QString typesPath = QFINDTESTDATA("data/servicetypes/example-servicetype.desktop");
        QVERIFY(!typesPath.isEmpty());
        const QString inputPath = QFINDTESTDATA("data/servicetypes/example-input.desktop");
        QVERIFY(!inputPath.isEmpty());
        QTest::ignoreMessage(
            QtWarningMsg,
            // We also print out a list of paths we searched in. With the ".+" we ensure that they are printed out,
            // but don't make fragile assumptions on the exact message
            QRegularExpression(QStringLiteral("Unable to find service type for service \"bar/foo\" listed in \"") + inputPath + QLatin1String("\" .+")));
        KPluginMetaData md = KPluginMetaData::fromDesktopFile(inputPath, QStringList() << typesPath);
        QVERIFY(md.isValid());
        QCOMPARE(md.name(), QStringLiteral("Example"));
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 89)
        QCOMPARE(md.serviceTypes(), QStringList() << QStringLiteral("example/servicetype") << QStringLiteral("bar/foo"));
#endif
        QCOMPARE(md.rawData().size(), 8);
        QVERIFY(md.rawData().value(QStringLiteral("KPlugin")).isObject());
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-Integer")), QJsonValue(42));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-Bool")), QJsonValue(true));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-Double")), QJsonValue(42.42));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-String")), QJsonValue(QStringLiteral("foobar")));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-List")),
                 QJsonValue(
                     QJsonArray::fromStringList(QStringList() << QStringLiteral("a") << QStringLiteral("b") << QStringLiteral("c") << QStringLiteral("def"))));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-Size")), QJsonValue(QStringLiteral("10,20"))); // QSize no longer supported (and also no longer used)
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-Unknown")), QJsonValue(QStringLiteral("true"))); // unknown property -> string
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 88)
        const QString charOverloadVlaue = md.value(QStringLiteral("X-Test-Unknown"), "true");
        QCOMPARE(charOverloadVlaue, QStringLiteral("true"));
#endif
    }

    void testBadGroupsInServiceType()
    {
        if (!doMessagesWork()) {
            return;
        }
        const QString typesPath = QFINDTESTDATA("data/servicetypes/bad-groups-servicetype.desktop");
        QVERIFY(!typesPath.isEmpty());
        const QString inputPath = QFINDTESTDATA("data/servicetypes/bad-groups-input.desktop");
        QVERIFY(!inputPath.isEmpty());
        QTest::ignoreMessage(QtWarningMsg, "Illegal .desktop group definition (does not end with ']'): \"[PropertyDef::MissingTerminator\"");
        QTest::ignoreMessage(QtWarningMsg, "Illegal .desktop group definition (does not end with ']'): \"[PropertyDef::\"");
        QTest::ignoreMessage(QtWarningMsg, "Illegal .desktop group definition (does not end with ']'): \"[\"");
        QTest::ignoreMessage(QtWarningMsg, "Read empty .desktop file group name! Invalid file?");
        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression(QStringLiteral("Skipping invalid group \"\" in service type \".*/bad-groups-servicetype.desktop\"")));
        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression(QStringLiteral("Skipping invalid group \"DoesNotStartWithPropertyDef::SomeOtherProperty\" in service type "
                                                               "\".+/data/servicetypes/bad-groups-servicetype.desktop\"")));
        QTest::ignoreMessage(QtWarningMsg, "Could not find Type= key in group \"PropertyDef::MissingType\"");
        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression(QStringLiteral("Property type \"integer\" is not a known QVariant type. Found while parsing property "
                                                               "definition for \"InvalidType\" in \".+/data/servicetypes/bad-groups-servicetype.desktop\"")));
        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression(QStringLiteral(".+/data/servicetypes/bad-groups-input.desktop:\\d+: Key name is missing: \"=11\"")));
        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression(QStringLiteral(".+/data/servicetypes/bad-groups-input.desktop:\\d+: Key name is missing: \"=13\"")));
        QTest::ignoreMessage(QtWarningMsg,
                             QRegularExpression(QStringLiteral(".+/data/servicetypes/bad-groups-input.desktop:\\d+: Key name is missing: \"=14\"")));
        KPluginMetaData md = KPluginMetaData::fromDesktopFile(inputPath, QStringList() << typesPath);
        QVERIFY(md.isValid());
        QCOMPARE(md.name(), QStringLiteral("Bad Groups"));
        QCOMPARE(md.rawData().size(), 8);
        QCOMPARE(md.rawData().value(QStringLiteral("ThisIsOkay")), QJsonValue(10)); // integer
        // 11 is empty group
        QCOMPARE(md.rawData().value(QStringLiteral("MissingTerminator")), QJsonValue(12)); // accept missing group terminator (for now) -> integer
        // 13 is empty group name
        // 14 is empty group name
        QCOMPARE(md.rawData().value(QStringLiteral("SomeOtherProperty")),
                 QJsonValue(QStringLiteral("15"))); // does not start with PropertyDef:: -> fall back to string
        QCOMPARE(md.rawData().value(QStringLiteral("TrailingSpacesAreOkay")), QJsonValue(16)); // accept trailing spaces in group name -> integer
        QCOMPARE(md.rawData().value(QStringLiteral("MissingType")), QJsonValue(QStringLiteral("17"))); // Type= missing -> fall back to string
        QCOMPARE(md.rawData().value(QStringLiteral("InvalidType")), QJsonValue(QStringLiteral("18"))); // Type= is invalid -> fall back to string
        QCOMPARE(md.rawData().value(QStringLiteral("ThisIsOkayAgain")), QJsonValue(19)); // valid definition after invalid ones should still work -> integer
    }
#endif

    void testJSONMetadata()
    {
        const QString inputPath = QFINDTESTDATA("data/testmetadata.json");
        KPluginMetaData md = KPluginMetaData::fromJsonFile(inputPath);
        QVERIFY(md.isValid());
        QCOMPARE(md.name(), QStringLiteral("Test"));

        QCOMPARE(md.value(QStringLiteral("X-Plasma-MainScript")), QStringLiteral("ui/main.qml"));
        QJsonArray expected;
        expected.append(QStringLiteral("Export"));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Purpose-PluginTypes")).toArray(), expected);
        QCOMPARE(md.value(QStringLiteral("SomeInt"), 24), 42);
        QCOMPARE(md.value(QStringLiteral("SomeIntAsString"), 24), 42);
        QCOMPARE(md.value(QStringLiteral("SomeStringNotAInt"), 24), 24);
        QCOMPARE(md.value(QStringLiteral("DoesNotExist"), 24), 24);

        QVERIFY(md.value(QStringLiteral("SomeBool"), false));
        QVERIFY(!md.value(QStringLiteral("SomeBoolThatIsFalse"), true));
        QVERIFY(md.value(QStringLiteral("SomeBoolAsString"), false));
        QVERIFY(md.value(QStringLiteral("DoesNotExist"), true));
    }

    void testPathIsAbsolute_data()
    {
        QTest::addColumn<QString>("inputAbsolute");
        QTest::addColumn<QString>("pluginPath");

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 91)
        // The .desktop file has X-KDE-Library, so .fileName() returns  different file
        QTest::newRow("desktop") << QFINDTESTDATA("data/fakeplugin.desktop") << QStringLiteral("fakeplugin");
#endif
        // But for the .json based plugin both are the same.
        QTest::newRow("json") << QFINDTESTDATA("data/testmetadata.json") << QFINDTESTDATA("data/testmetadata.json");
        // And also for the library with embedded JSON metadata.
        QPluginLoader shlibLoader(QCoreApplication::applicationDirPath() + QStringLiteral("/jsonplugin"));
        QVERIFY2(!shlibLoader.fileName().isEmpty(), "Could not find jsonplugin");
        QString shlibPath = QFileInfo(shlibLoader.fileName()).absoluteFilePath();
        QTest::newRow("library") << shlibPath << shlibPath;
    }

    void testPathIsAbsolute()
    {
        // Test that the fileName() accessor always returns an absolute path if it was used.
        QFETCH(QString, inputAbsolute);
        QVERIFY2(QDir::isAbsolutePath(inputAbsolute), qPrintable(inputAbsolute));
        QFETCH(QString, pluginPath);

        const auto createMetaData = [](const QString &path) {
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 91)
            return KPluginMetaData(path);
#else
            if (path.endsWith(QLatin1String(".json"))) {
                return KPluginMetaData::fromJsonFile(path);
            } else {
                return KPluginMetaData(path);
            }
#endif
        };

        KPluginMetaData mdAbsolute = createMetaData(inputAbsolute);
        QVERIFY(mdAbsolute.isValid());
        QCOMPARE(mdAbsolute.metaDataFileName(), inputAbsolute);
        QCOMPARE(mdAbsolute.fileName(), pluginPath);

        // All files that have been opened should be stored as absolute paths.
        QString inputRelative;
        if (QLibrary::isLibrary(inputAbsolute)) {
            // We have a plugin without namespace, with the code path below we would end up with
            // a path relative to the PWD, but we want to check a path relative to the plugin dir.
            // Because of that we simply use the baseName of the file.
            inputRelative = QFileInfo(inputAbsolute).baseName();
        } else {
            inputRelative = QDir::current().relativeFilePath(inputAbsolute);
        }
        QVERIFY2(QDir::isRelativePath(inputRelative), qPrintable(inputRelative));
        KPluginMetaData mdRelative = createMetaData(inputRelative);
        QVERIFY(mdRelative.isValid());
        QCOMPARE(mdRelative.metaDataFileName(), inputAbsolute);
        QCOMPARE(mdRelative.fileName(), pluginPath);

        // Check that creating it with the parsed JSON object and a path keeps the path unchanged
        const QJsonObject json = mdAbsolute.rawData();
        QString pluginRelative = QDir::current().relativeFilePath(pluginPath);
        QVERIFY2(QDir::isRelativePath(pluginRelative), qPrintable(pluginRelative));
        // TODO: KF6: no need to test both constructors once they are merged into one overload.
        KPluginMetaData mdFromJson1(json, pluginRelative, inputRelative);
        QCOMPARE(mdFromJson1.metaDataFileName(), inputRelative);
        // We should not be normalizing files that have not been openened, so both arguments should be unchanged.
        QCOMPARE(mdFromJson1.fileName(), pluginRelative);
        KPluginMetaData mdFromJson2(json, inputRelative);
        QCOMPARE(mdFromJson2.metaDataFileName(), inputRelative);
        QCOMPARE(mdFromJson2.fileName(), inputRelative);
    }

    void testFindPlugins()
    {
        QTemporaryDir temp;
        QVERIFY(temp.isValid());
        QDir dir(temp.path());
        QVERIFY(dir.mkdir(QStringLiteral("kpluginmetadatatest")));
        QVERIFY(dir.cd(QStringLiteral("kpluginmetadatatest")));
        for (const QString &name : {QStringLiteral("jsonplugin"), QStringLiteral("unversionedplugin"), QStringLiteral("jsonplugin2")}) {
            const QString pluginPath = QPluginLoader(name).fileName();
            QVERIFY2(!pluginPath.isEmpty(), qPrintable(pluginPath));
            QVERIFY2(QFile::copy(pluginPath, dir.absoluteFilePath(QFileInfo(pluginPath).fileName())),
                     qPrintable(dir.absoluteFilePath(QFileInfo(pluginPath).fileName())));
        }

        LibraryPathRestorer restorer(QCoreApplication::libraryPaths());
        // we only want plugins from our temporary dir
        QCoreApplication::setLibraryPaths(QStringList() << temp.path());

        auto sortPlugins = [](const KPluginMetaData &a, const KPluginMetaData &b) {
            return a.pluginId() < b.pluginId();
        };
        // it should find jsonplugin and jsonplugin2 since unversionedplugin does not have any meta data
        auto plugins = KPluginMetaData::findPlugins(QStringLiteral("kpluginmetadatatest"));
        std::sort(plugins.begin(), plugins.end(), sortPlugins);
        QCOMPARE(plugins.size(), 2);
        QCOMPARE(plugins[0].pluginId(), QStringLiteral("foobar")); // ID is not the filename, it is set in the JSON metadata
        QCOMPARE(plugins[0].description(), QStringLiteral("This is another plugin"));
        QCOMPARE(plugins[1].pluginId(), QStringLiteral("jsonplugin"));
        QCOMPARE(plugins[1].description(), QStringLiteral("This is a plugin"));

        // filter accepts none
        plugins = KPluginMetaData::findPlugins(QStringLiteral("kpluginmetadatatest"), [](const KPluginMetaData &) {
            return false;
        });
        std::sort(plugins.begin(), plugins.end(), sortPlugins);
        QCOMPARE(plugins.size(), 0);

        // filter accepts all
        plugins = KPluginMetaData::findPlugins(QStringLiteral("kpluginmetadatatest"), [](const KPluginMetaData &) {
            return true;
        });
        std::sort(plugins.begin(), plugins.end(), sortPlugins);
        QCOMPARE(plugins.size(), 2);
        QCOMPARE(plugins[0].description(), QStringLiteral("This is another plugin"));
        QCOMPARE(plugins[1].description(), QStringLiteral("This is a plugin"));

        // mimetype filter. Only one match, jsonplugin2 is specific to text/html.
        auto supportTextPlain = [](const KPluginMetaData &metaData) {
            return metaData.supportsMimeType(QLatin1String("text/plain"));
        };
        plugins = KPluginMetaData::findPlugins(QStringLiteral("kpluginmetadatatest"), supportTextPlain);
        QCOMPARE(plugins.size(), 1);
        QCOMPARE(plugins[0].description(), QStringLiteral("This is a plugin"));

        // mimetype filter. Two matches, both support text/html, via inheritance.
        auto supportTextHtml = [](const KPluginMetaData &metaData) {
            return metaData.supportsMimeType(QLatin1String("text/html"));
        };
        plugins = KPluginMetaData::findPlugins(QStringLiteral("kpluginmetadatatest"), supportTextHtml);
        std::sort(plugins.begin(), plugins.end(), sortPlugins);
        QCOMPARE(plugins.size(), 2);
        QCOMPARE(plugins[0].description(), QStringLiteral("This is another plugin"));
        QCOMPARE(plugins[1].description(), QStringLiteral("This is a plugin"));

        // mimetype filter with invalid mimetype
        auto supportDoesNotExist = [](const KPluginMetaData &metaData) {
            return metaData.supportsMimeType(QLatin1String("does/not/exist"));
        };
        plugins = KPluginMetaData::findPlugins(QStringLiteral("kpluginmetadatatest"), supportDoesNotExist);
        QCOMPARE(plugins.size(), 0);

        // invalid std::function as filter
        plugins = KPluginMetaData::findPlugins(QStringLiteral("kpluginmetadatatest"));
        std::sort(plugins.begin(), plugins.end(), sortPlugins);
        QCOMPARE(plugins.size(), 2);
        QCOMPARE(plugins[0].description(), QStringLiteral("This is another plugin"));
        QCOMPARE(plugins[1].description(), QStringLiteral("This is a plugin"));

        // by plugin id
        KPluginMetaData plugin = KPluginMetaData::findPluginById(dir.absolutePath(), QStringLiteral("foobar"));
        QVERIFY(plugin.isValid());
        QCOMPARE(plugin.description(), QStringLiteral("This is another plugin"));

        // by plugin invalid id
        plugin = KPluginMetaData::findPluginById(dir.absolutePath(), QStringLiteral("invalidid"));
        QVERIFY(!plugin.isValid());

        // absolute path, no filter
        plugins = KPluginMetaData::findPlugins(dir.absolutePath());
        std::sort(plugins.begin(), plugins.end(), sortPlugins);
        QCOMPARE(plugins.size(), 2);
        QCOMPARE(plugins[0].description(), QStringLiteral("This is another plugin"));
        QCOMPARE(plugins[1].description(), QStringLiteral("This is a plugin"));

        // This plugin has no explicit pluginId and will fall back to basename of file
        const KPluginMetaData validPlugin = KPluginMetaData::findPluginById(dir.absolutePath(), QStringLiteral("jsonplugin"));
        QVERIFY(validPlugin.isValid());
        QCOMPARE(plugins[0].description(), QStringLiteral("This is another plugin"));

        // The basename matches, but the pluginId does not match
        const KPluginMetaData nonMatchingPluginId = KPluginMetaData::findPluginById(dir.absolutePath(), QStringLiteral("jsonplugin2"));
        QVERIFY(!nonMatchingPluginId.isValid());

        const KPluginMetaData nonExistingPlugin = KPluginMetaData::findPluginById(dir.absolutePath(), QStringLiteral("invalidid"));
        QVERIFY(!nonExistingPlugin.isValid());
    }

    void testStaticPlugins()
    {
        QCOMPARE(QPluginLoader::staticPlugins().count(), 0);

        const auto plugins = KPluginMetaData::findPlugins(QStringLiteral("staticnamespace"));
        QCOMPARE(plugins.count(), 1);

        QCOMPARE(plugins.first().description(), QStringLiteral("This is a plugin"));
        QCOMPARE(plugins.first().fileName(), QStringLiteral("staticnamespace/static_jsonplugin_cmake_macro"));
    }

    void testPluginsWithoutMetaData()
    {
        KPluginMetaData emptyMetaData(QStringLiteral("namespace/pluginwithoutmetadata"), KPluginMetaData::AllowEmptyMetaData);
        QVERIFY(emptyMetaData.isValid());
        QCOMPARE(emptyMetaData.pluginId(), QStringLiteral("pluginwithoutmetadata"));

        const auto plugins = KPluginMetaData::findPlugins(QStringLiteral("namespace"), {}, KPluginMetaData::AllowEmptyMetaData);
        QCOMPARE(plugins.count(), 2);
        for (auto plugin : plugins) {
            if (plugin.pluginId() == QLatin1String("pluginwithoutmetadata")) {
                QVERIFY(plugin.isValid());
                QVERIFY(plugin.rawData().isEmpty());
            } else if (plugin.pluginId() == QLatin1String("jsonplugin_cmake_macro")) {
                QVERIFY(plugin.isValid());
                QVERIFY(!plugin.rawData().isEmpty());
            } else {
                Q_UNREACHABLE();
            }
        }
    }

    void testStaticPluginsWithoutMetadata()
    {
        QVERIFY(KPluginMetaData::findPlugins(QStringLiteral("staticnamespace3")).isEmpty());
        const auto plugins = KPluginMetaData::findPlugins(QStringLiteral("staticnamespace3"), {}, KPluginMetaData::AllowEmptyMetaData);
        QCOMPARE(plugins.count(), 1);
        QVERIFY(plugins.first().isValid());
        QCOMPARE(plugins.first().pluginId(), QStringLiteral("static_plugin_without_metadata"));
    }

    void testReverseDomainNotationPluginId()
    {
        KPluginMetaData data(QStringLiteral("org.kde.test"));
        QVERIFY(data.isValid());
        QCOMPARE(data.pluginId(), QStringLiteral("org.kde.test"));
    }

    void testFindingPluginInAppDirFirst()
    {
        const QString originalPluginPath = QPluginLoader(QStringLiteral("namespace/jsonplugin_cmake_macro")).fileName();
        const QString pluginFileName = QFileInfo(originalPluginPath).fileName();
        const QString pluginNamespace = QStringLiteral("somepluginnamespace");
        const QString pluginAppDir = QCoreApplication::applicationDirPath() + QLatin1Char('/') + pluginNamespace;
        QDir(pluginAppDir).mkpath(QStringLiteral("."));
        const QString pluginAppPath = pluginAppDir + QLatin1Char('/') + pluginFileName;
        QFile::remove(pluginAppPath);

        QVERIFY(QFile::copy(originalPluginPath, pluginAppPath));

        QTemporaryDir temp;
        QVERIFY(temp.isValid());
        QDir dir(temp.path());
        QVERIFY(dir.mkdir(pluginNamespace));
        QVERIFY(dir.cd(pluginNamespace));

        const QString pluginInNamespacePath = dir.absoluteFilePath(pluginFileName);
        QVERIFY(QFile::copy(originalPluginPath, pluginInNamespacePath));

        LibraryPathRestorer restorer(QCoreApplication::libraryPaths());
        QCoreApplication::setLibraryPaths(QStringList() << temp.path());

        // Our plugin in the applicationDirPath should come first
        const QString relativePathWithNamespace = QStringLiteral("somepluginnamespace/jsonplugin_cmake_macro");
        KPluginMetaData data(relativePathWithNamespace);
        QVERIFY(data.isValid());
        QCOMPARE(data.fileName(), pluginAppPath);

        // The other one must be valid
        QVERIFY(KPluginMetaData(pluginInNamespacePath).isValid());
        // And after removing the plugin in the applicationDirPath, it should be found
        QVERIFY(QFile::remove(pluginAppPath));
        QCOMPARE(KPluginMetaData(relativePathWithNamespace).fileName(), pluginInNamespacePath);
    }
};

QTEST_MAIN(KPluginMetaDataTest)

#include "kpluginmetadatatest.moc"
