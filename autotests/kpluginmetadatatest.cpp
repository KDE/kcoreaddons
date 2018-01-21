/*
 * Copyright 2014 Alex Richardson <arichardson.kde@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <QtTest>

#include <kpluginmetadata.h>
#include <kpluginloader.h>
#include <kaboutdata.h>

#include <QLocale>


namespace QTest
{
template<> inline char *toString(const QJsonValue &val)
{
    // simply reuse the QDebug representation
    QString result;
    QDebug(&result) << val;
    return QTest::toString(result);
}
}

class KPluginMetaDataTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testFromPluginLoader()
    {
        QString location = KPluginLoader::findPlugin(QStringLiteral("jsonplugin"));
        QVERIFY2(!location.isEmpty(),"Could not find jsonplugin");

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
        KPluginMetaData fromKPluginLoader(KPluginLoader(QStringLiteral("jsonplugin")));
        KPluginMetaData fromFullPath(location);
        KPluginMetaData fromRelativePath(QStringLiteral("jsonplugin"));
        KPluginMetaData fromRawData(jsonDoc.object(), location);

        auto description = QStringLiteral("This is a plugin");

        QVERIFY(fromQPluginLoader.isValid());
        QCOMPARE(fromQPluginLoader.description(), description);
        QVERIFY(fromKPluginLoader.isValid());
        QCOMPARE(fromKPluginLoader.description(), description);
        QVERIFY(fromFullPath.isValid());
        QCOMPARE(fromFullPath.description(), description);
        QVERIFY(fromRelativePath.isValid());
        QCOMPARE(fromRelativePath.description(), description);
        QVERIFY(fromRawData.isValid());
        QCOMPARE(fromRawData.description(), description);

        // check operator==
        QCOMPARE(fromRawData, fromRawData);
        QCOMPARE(fromQPluginLoader, fromQPluginLoader);
        QCOMPARE(fromKPluginLoader, fromKPluginLoader);
        QCOMPARE(fromFullPath, fromFullPath);

        QCOMPARE(fromQPluginLoader, fromKPluginLoader);
        QCOMPARE(fromQPluginLoader, fromFullPath);
        QCOMPARE(fromQPluginLoader, fromRawData);

        QCOMPARE(fromKPluginLoader, fromQPluginLoader);
        QCOMPARE(fromKPluginLoader, fromFullPath);
        QCOMPARE(fromKPluginLoader, fromRawData);

        QCOMPARE(fromFullPath, fromQPluginLoader);
        QCOMPARE(fromFullPath, fromKPluginLoader);
        QCOMPARE(fromFullPath, fromRawData);

        QCOMPARE(fromRawData, fromQPluginLoader);
        QCOMPARE(fromRawData, fromKPluginLoader);
        QCOMPARE(fromRawData, fromFullPath);

    }

    void testAllKeys()
    {
        QJsonParseError e;
        QJsonObject jo = QJsonDocument::fromJson("{\n"
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
                                                 " }\n}\n", &e).object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        KPluginMetaData m(jo, QString());
        QVERIFY(m.isValid());
        QCOMPARE(m.pluginId(), QStringLiteral("time"));
        QCOMPARE(m.name(), QStringLiteral("Date and Time"));
        QCOMPARE(m.description(), QStringLiteral("Date and time by timezone"));
        QCOMPARE(m.extraInformation(), QStringLiteral("Something else"));
        QCOMPARE(m.iconName(), QStringLiteral("preferences-system-time"));
        QCOMPARE(m.category(), QStringLiteral("Date and Time"));
        QCOMPARE(m.dependencies(), QStringList() << QStringLiteral("foo") << QStringLiteral("bar"));
        QCOMPARE(m.authors().size(), 1);
        QCOMPARE(m.authors()[0].name(), QStringLiteral("Aaron Seigo"));
        QCOMPARE(m.authors()[0].emailAddress(), QStringLiteral("aseigo@kde.org"));
        QCOMPARE(m.translators().size(), 1);
        QCOMPARE(m.translators()[0].name(), QStringLiteral("No One"));
        QCOMPARE(m.translators()[0].emailAddress(), QStringLiteral("no.one@kde.org"));
        QCOMPARE(m.otherContributors().size(), 1);
        QCOMPARE(m.otherContributors()[0].name(), QStringLiteral("No One"));
        QCOMPARE(m.otherContributors()[0].emailAddress(), QStringLiteral("no.one@kde.org"));
        QVERIFY(m.isEnabledByDefault());
        QCOMPARE(m.license(), QStringLiteral("LGPL"));
        QCOMPARE(m.copyrightText(), QStringLiteral("(c) Alex Richardson 2015"));
        QCOMPARE(m.version(), QStringLiteral("1.0"));
        QCOMPARE(m.website(), QStringLiteral("https://plasma.kde.org/"));
        QCOMPARE(m.serviceTypes(), QStringList() << QStringLiteral("Plasma/DataEngine"));
        QCOMPARE(m.mimeTypes(), QStringList() << QStringLiteral("image/png"));
    }

    void testTranslations()
    {
        QJsonParseError e;
        QJsonObject jo = QJsonDocument::fromJson("{ \"KPlugin\": {\n"
                                                 "\"Name\": \"Name\",\n"
                                                 "\"Name[de]\": \"Name (de)\",\n"
                                                 "\"Name[de_DE]\": \"Name (de_DE)\",\n"
                                                 "\"Description\": \"Description\",\n"
                                                 "\"Description[de]\": \"Beschreibung (de)\",\n"
                                                 "\"Description[de_DE]\": \"Beschreibung (de_DE)\"\n"
                                                 "}\n}", &e).object();
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
        QJsonParseError e;
        QJsonObject jo = QJsonDocument::fromJson("{\n"
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
                                                 "}", &e).object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        QTest::ignoreMessage(QtWarningMsg, "Expected JSON property \"String\" to be a string list. Treating it as a list with a single entry: \"foo\" ");
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("String")), QStringList(QStringLiteral("foo")));;
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("OneArrayEntry")), QStringList(QStringLiteral("foo")));
        QTest::ignoreMessage(QtWarningMsg, "Expected JSON property \"Bool\" to be a string list. Treating it as a list with a single entry: \"true\" ");
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("Bool")), QStringList(QStringLiteral("true")));
        QTest::ignoreMessage(QtWarningMsg, "Expected JSON property \"QuotedBool\" to be a string list. Treating it as a list with a single entry: \"true\" ");
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("QuotedBool")), QStringList(QStringLiteral("true")));
        QTest::ignoreMessage(QtWarningMsg, "Expected JSON property \"Number\" to be a string list. Treating it as a list with a single entry: \"12345\" ");
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("Number")), QStringList(QStringLiteral("12345")));
        QTest::ignoreMessage(QtWarningMsg, "Expected JSON property \"QuotedNumber\" to be a string list. Treating it as a list with a single entry: \"12345\" ");
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("QuotedNumber")), QStringList(QStringLiteral("12345")));
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("EmptyArray")), QStringList());
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("NumberArray")), QStringList() << QStringLiteral("1") << QStringLiteral("2") << QStringLiteral("3"));
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("BoolArray")), QStringList() << QStringLiteral("true") << QStringLiteral("false") << QStringLiteral("true"));
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("StringArray")), QStringList() << QStringLiteral("foo") << QStringLiteral("bar"));
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("Null")), QStringList());
        QTest::ignoreMessage(QtWarningMsg, "Expected JSON property \"QuotedNull\" to be a string list. Treating it as a list with a single entry: \"null\" ");
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("QuotedNull")), QStringList(QStringLiteral("null")));
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("ArrayWithNull")), QStringList() << QStringLiteral("foo") << QStringLiteral("") << QStringLiteral("bar"));
        QCOMPARE(KPluginMetaData::readStringList(jo, QStringLiteral("Object")), QStringList());
    }

    void testFromDesktopFile()
    {
        const QString dfile = QFINDTESTDATA("data/fakeplugin.desktop");
        KPluginMetaData md(dfile);
        QVERIFY(md.isValid());
        QCOMPARE(md.pluginId(), QStringLiteral("fakeplugin"));
        QCOMPARE(md.fileName(),  QStringLiteral("fakeplugin"));
        QCOMPARE(md.metaDataFileName(), dfile);
        QCOMPARE(md.iconName(), QStringLiteral("preferences-system-time"));
        QCOMPARE(md.license(), QStringLiteral("LGPL"));
        QCOMPARE(md.website(), QStringLiteral("https://kde.org/"));
        QCOMPARE(md.category(), QStringLiteral("Examples"));
        QCOMPARE(md.version(), QStringLiteral("1.0"));
        QCOMPARE(md.dependencies(), QStringList());
        QCOMPARE(md.isHidden(), false);
        QCOMPARE(md.serviceTypes(), QStringList(QStringLiteral("KService/NSA")));
        QCOMPARE(md.mimeTypes(), QStringList() << QStringLiteral("image/png") << QStringLiteral("application/pdf"));

        auto kp = md.rawData()[QStringLiteral("KPlugin")].toObject();
        QStringList formFactors = KPluginMetaData::readStringList(kp, QStringLiteral("FormFactors"));
        QCOMPARE(formFactors, QStringList() << QStringLiteral("mediacenter") << QStringLiteral("desktop"));
        QCOMPARE(md.formFactors(), QStringList() << QStringLiteral("mediacenter") << QStringLiteral("desktop"));

        const QString dfilehidden = QFINDTESTDATA("data/hiddenplugin.desktop");
        KPluginMetaData mdhidden(dfilehidden);
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
        QStringList list = KPluginMetaData::readStringList(md.rawData(), QStringLiteral("X-Test-List"));
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
        const QString typesPath = QFINDTESTDATA("data/servicetypes/example-servicetype.desktop");
        QVERIFY(!typesPath.isEmpty());
        const QString inputPath = QFINDTESTDATA("data/servicetypes/example-input.desktop");
        QVERIFY(!inputPath.isEmpty());
        KPluginMetaData md = KPluginMetaData::fromDesktopFile(inputPath, QStringList() << typesPath);
        QVERIFY(md.isValid());
        QCOMPARE(md.name(), QStringLiteral("Example"));
        QCOMPARE(md.serviceTypes(), QStringList() << QStringLiteral("foo/bar") << QStringLiteral("bar/foo"));
        // qDebug().noquote() << QJsonDocument(md.rawData()).toJson();
        QCOMPARE(md.rawData().size(), 8);
        QVERIFY(md.rawData().value(QStringLiteral("KPlugin")).isObject());
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-Integer")), QJsonValue(42));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-Bool")), QJsonValue(true));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-Double")), QJsonValue(42.42));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-String")), QJsonValue(QStringLiteral("foobar")));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-List")), QJsonValue(QJsonArray::fromStringList(QStringList() << QStringLiteral("a") << QStringLiteral("b") << QStringLiteral("c") << QStringLiteral("def"))));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-Size")), QJsonValue(QStringLiteral("10,20"))); // QSize no longer supported (and also no longer used)
        QCOMPARE(md.rawData().value(QStringLiteral("X-Test-Unknown")), QJsonValue(QStringLiteral("true"))); // unknown property -> string

    }

    void testBadGroupsInServiceType()
    {
        const QString typesPath = QFINDTESTDATA("data/servicetypes/bad-groups-servicetype.desktop");
        QVERIFY(!typesPath.isEmpty());
        const QString inputPath = QFINDTESTDATA("data/servicetypes/bad-groups-input.desktop");
        QVERIFY(!inputPath.isEmpty());
        QTest::ignoreMessage(QtWarningMsg, "Illegal .desktop group definition (does not end with ']'): \"[PropertyDef::MissingTerminator\"");
        QTest::ignoreMessage(QtWarningMsg, "Illegal .desktop group definition (does not end with ']'): \"[PropertyDef::\"");
        QTest::ignoreMessage(QtWarningMsg, "Illegal .desktop group definition (does not end with ']'): \"[\"");
        QTest::ignoreMessage(QtWarningMsg, "Read empty .desktop file group name! Invalid file?");
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Skipping invalid group \"\" in service type \".*/bad-groups-servicetype.desktop\"")));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Skipping invalid group \"DoesNotStartWithPropertyDef::SomeOtherProperty\" in service type \".+/data/servicetypes/bad-groups-servicetype.desktop\"")));
        QTest::ignoreMessage(QtWarningMsg, "Could not find Type= key in group \"PropertyDef::MissingType\"");
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral("Property type \"integer\" is not a known QVariant type. Found while parsing property definition for \"InvalidType\" in \".+/data/servicetypes/bad-groups-servicetype.desktop\"")));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral(".+/data/servicetypes/bad-groups-input.desktop:\\d+: Key name is missing: \"=11\"")));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral(".+/data/servicetypes/bad-groups-input.desktop:\\d+: Key name is missing: \"=13\"")));
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(QStringLiteral(".+/data/servicetypes/bad-groups-input.desktop:\\d+: Key name is missing: \"=14\"")));
        KPluginMetaData md = KPluginMetaData::fromDesktopFile(inputPath, QStringList() << typesPath);
        QVERIFY(md.isValid());
        QCOMPARE(md.name(), QStringLiteral("Bad Groups"));
        // qDebug().noquote() << QJsonDocument(md.rawData()).toJson();
        QCOMPARE(md.rawData().size(), 8);
        QCOMPARE(md.rawData().value(QStringLiteral("ThisIsOkay")), QJsonValue(10)); // integer
        // 11 is empty group
        QCOMPARE(md.rawData().value(QStringLiteral("MissingTerminator")), QJsonValue(12)); // accept missing group terminator (for now) -> integer
        // 13 is empty group name
        // 14 is empty group name
        QCOMPARE(md.rawData().value(QStringLiteral("SomeOtherProperty")), QJsonValue(QStringLiteral("15"))); // does not start with PropertyDef:: -> fall back to string
        QCOMPARE(md.rawData().value(QStringLiteral("TrailingSpacesAreOkay")), QJsonValue(16)); // accept trailing spaces in group name -> integer
        QCOMPARE(md.rawData().value(QStringLiteral("MissingType")), QJsonValue(QStringLiteral("17"))); // Type= missing -> fall back to string
        QCOMPARE(md.rawData().value(QStringLiteral("InvalidType")), QJsonValue(QStringLiteral("18"))); // Type= is invalid -> fall back to string
        QCOMPARE(md.rawData().value(QStringLiteral("ThisIsOkayAgain")), QJsonValue(19)); // valid defintion after invalid ones should still work -> integer
    }

    void testJSONMetadata()
    {
        const QString inputPath = QFINDTESTDATA("data/testmetadata.json");
        KPluginMetaData md(inputPath);
        QVERIFY(md.isValid());
        QCOMPARE(md.name(), QStringLiteral("Test"));

        QCOMPARE(md.value(QStringLiteral("X-Plasma-MainScript")), QStringLiteral("ui/main.qml"));
        QJsonArray expected;
        expected.append(QStringLiteral("Export"));
        QCOMPARE(md.rawData().value(QStringLiteral("X-Purpose-PluginTypes")).toArray(), expected);
    }
};

QTEST_MAIN(KPluginMetaDataTest)

#include "kpluginmetadatatest.moc"
