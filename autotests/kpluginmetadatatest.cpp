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

class KPluginMetaDataTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testFromPluginLoader()
    {
        QString location = KPluginLoader::findPlugin("jsonplugin");
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

        KPluginMetaData fromQPluginLoader(QPluginLoader("jsonplugin"));
        KPluginMetaData fromKPluginLoader(KPluginLoader("jsonplugin"));
        KPluginMetaData fromFullPath(location);
        KPluginMetaData fromRelativePath("jsonplugin");
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
                " \"Category\": \"Date and Time\",\n"
                " \"Dependencies\": [ \"foo\", \"bar\"],\n"
                " \"EnabledByDefault\": \"true\",\n"
                " \"License\": \"LGPL\",\n"
                " \"Id\": \"time\",\n"
                " \"Version\": \"1.0\",\n"
                " \"Website\": \"http://plasma.kde.org/\",\n"
                " \"ServiceTypes\": [\"Plasma/DataEngine\"]\n"
            " }\n}\n", &e).object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        KPluginMetaData m(jo, QString());
        QVERIFY(m.isValid());
        QCOMPARE(m.pluginId(), QStringLiteral("time"));
        QCOMPARE(m.name(), QStringLiteral("Date and Time"));
        QCOMPARE(m.description(), QStringLiteral("Date and time by timezone"));
        QCOMPARE(m.iconName(), QStringLiteral("preferences-system-time"));
        QCOMPARE(m.category(), QStringLiteral("Date and Time"));
        QCOMPARE(m.dependencies(), QStringList() << "foo" << "bar");
        QCOMPARE(m.authors().size(), 1);
        QCOMPARE(m.authors()[0].name(), QStringLiteral("Aaron Seigo"));
        QCOMPARE(m.authors()[0].emailAddress(), QStringLiteral("aseigo@kde.org"));
        QVERIFY(m.isEnabledByDefault());
        QCOMPARE(m.license(), QStringLiteral("LGPL"));
        QCOMPARE(m.version(), QStringLiteral("1.0"));
        QCOMPARE(m.website(), QStringLiteral("http://plasma.kde.org/"));
        QCOMPARE(m.serviceTypes(), QStringList() << QStringLiteral("Plasma/DataEngine"));
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

        QLocale::setDefault(QLocale("de_DE"));
        QCOMPARE(m.name(), QStringLiteral("Name (de_DE)"));
        QCOMPARE(m.description(), QStringLiteral("Beschreibung (de_DE)"));

        QLocale::setDefault(QLocale("de_CH"));
        QCOMPARE(m.name(), QStringLiteral("Name (de)"));
        QCOMPARE(m.description(), QStringLiteral("Beschreibung (de)"));

        QLocale::setDefault(QLocale("fr_FR"));
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
        QCOMPARE(KPluginMetaData::readStringList(jo, "String"), QStringList() << "foo");
        QCOMPARE(KPluginMetaData::readStringList(jo, "OneArrayEntry"), QStringList() << "foo");
        QCOMPARE(KPluginMetaData::readStringList(jo, "Bool"), QStringList() << "true");
        QCOMPARE(KPluginMetaData::readStringList(jo, "QuotedBool"), QStringList() << "true");
        QCOMPARE(KPluginMetaData::readStringList(jo, "Number"), QStringList() << "12345");
        QCOMPARE(KPluginMetaData::readStringList(jo, "QuotedNumber"), QStringList() << "12345");
        QCOMPARE(KPluginMetaData::readStringList(jo, "EmptyArray"), QStringList());
        QCOMPARE(KPluginMetaData::readStringList(jo, "NumberArray"), QStringList() << "1" << "2" << "3");
        QCOMPARE(KPluginMetaData::readStringList(jo, "BoolArray"), QStringList() << "true" << "false" << "true");
        QCOMPARE(KPluginMetaData::readStringList(jo, "StringArray"), QStringList() << "foo" << "bar");
        QCOMPARE(KPluginMetaData::readStringList(jo, "Null"), QStringList());
        QCOMPARE(KPluginMetaData::readStringList(jo, "QuotedNull"), QStringList() << "null");
        QCOMPARE(KPluginMetaData::readStringList(jo, "ArrayWithNull"), QStringList() << "foo" << "" << "bar");
        QCOMPARE(KPluginMetaData::readStringList(jo, "Object"), QStringList());
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
        QCOMPARE(md.website(), QStringLiteral("http://kde.org/"));
        QCOMPARE(md.category(), QStringLiteral("Examples"));
        QCOMPARE(md.version(), QStringLiteral("1.0"));
        QCOMPARE(md.dependencies(), QStringList());
        QCOMPARE(md.isHidden(), false);
        QCOMPARE(md.serviceTypes(), QStringList("KService/NSA"));

        const QString dfilehidden = QFINDTESTDATA("data/fakeplugin.desktop");
        KPluginMetaData mdhidden(dfilehidden);
        QVERIFY(md.isValid());
        QCOMPARE(md.isHidden(), true);
    }
};

QTEST_MAIN(KPluginMetaDataTest)

#include "kpluginmetadatatest.moc"
