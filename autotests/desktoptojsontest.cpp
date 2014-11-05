/* This file is part of the KDE project
   Copyright (C) 2014 Alex Richardson <arichardson.kde@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QObject>
#include <QProcess>
#include <QTemporaryFile>
#include <QTest>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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

class DesktopToJsonTest : public QObject
{
    Q_OBJECT

private:
    void compareJson(const QJsonObject& actual, const QJsonObject& expected) {
        for (auto it = actual.constBegin(); it != actual.constEnd(); ++it) {
            if (expected.constFind(it.key()) == expected.constEnd()) {
                qCritical() << "Result has key" << it.key() << "which is not expected!";
                QFAIL("Invalid output");
            }
            if (it.value().isObject() && expected.value(it.key()).isObject()) {
                compareJson(it.value().toObject(), expected.value(it.key()).toObject());
            } else {
                QCOMPARE(it.value(), expected.value(it.key()));
            }
        }
        for (auto it = expected.constBegin(); it != expected.constEnd(); ++it) {
            if (actual.constFind(it.key()) == actual.constEnd()) {
                qCritical() << "Result is missing key" << it.key();
                QFAIL("Invalid output");
            }
            if (it.value().isObject() && actual.value(it.key()).isObject()) {
                compareJson(it.value().toObject(), actual.value(it.key()).toObject());
            } else {
                QCOMPARE(it.value(), actual.value(it.key()));
            }
        }
    }

private Q_SLOTS:

    void testDesktopToJson_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<QJsonObject>("expectedResult");
        QTest::addColumn<bool>("compatibilityMode");

        QJsonObject expectedResult;
        QJsonObject kpluginObj;
        QByteArray input =
            // include an insignificant group
            "[Some Group]\n"
            "Foo=Bar\n"
            "\n"
            "[Desktop Entry]\n"
        // only data inside [Desktop Entry] should be included
            "Name=Example\n"
        //empty lines
            "\n"
            " \n"
            // make sure translations are included:
            "Name[de_DE]=Beispiel\n"
            // ignore comments:
            "#Comment=Comment\n"
            "  #Comment=Comment\n"
            "Categories=foo;bar;a\\;b\n"
        // As the case is significant, the keys Name and NAME are not equivalent:
            "CaseSensitive=ABC\n"
            "CASESENSITIVE=abc\n"
        // Space before and after the equals sign should be ignored:
            "SpacesBeforeEq   =foo\n"
            "SpacesAfterEq=   foo\n"
        //  Space before and after the equals sign should be ignored; the = sign is the actual delimiter.
        // TODO: error in spec (spaces before and after the key??)
            "   SpacesBeforeKey=foo\n"
            "SpacesAfterKey   =foo\n"
        // ignore trailing spaces
            "TrailingSpaces=foo   \n"
        // However spaces in the value are significant:
            "SpacesInValue=Hello, World!\n"
        //  The escape sequences \s, \n, \t, \r, and \\ are supported for values of
        // type string and localestring, meaning ASCII space, newline, tab,
        // carriage return, and backslash, respectively:
            "EscapeSequences=So\\sme esc\\nap\\te se\\\\qu\\re\\\\nces\n" // make sure that the last n is a literal n not a newline!
        // the standard keys that are used by plugins, make sure correct types are used:
            "X-KDE-PluginInfo-Category=Examples\n" // string key
            "X-KDE-PluginInfo-Version=1.0\n"
        // The multiple values should be separated by a semicolon and the value of the key
        // may be optionally terminated by a semicolon. Trailing empty strings must always
        // be terminated with a semicolon. Semicolons in these values need to be escaped using \;.
            "X-KDE-PluginInfo-Depends=foo,bar,esc\\,aped\n" // string list key
            "X-KDE-ServiceTypes=\n" // empty string list
            "X-KDE-PluginInfo-EnabledByDefault=true\n" // bool key
        // now start a new group
            "[New Group]\n"
            "InWrongGroup=true\n";

        expectedResult["Categories"] = QStringLiteral("foo;bar;a\\;b");
        expectedResult["CaseSensitive"] = QStringLiteral("ABC");
        expectedResult["CASESENSITIVE"] = QStringLiteral("abc");
        expectedResult["SpacesBeforeEq"] = QStringLiteral("foo");
        expectedResult["SpacesAfterEq"] = QStringLiteral("foo");
        expectedResult["SpacesBeforeKey"] = QStringLiteral("foo");
        expectedResult["SpacesAfterKey"] = QStringLiteral("foo");
        expectedResult["TrailingSpaces"] = QStringLiteral("foo");
        expectedResult["SpacesInValue"] = QStringLiteral("Hello, World!");
        expectedResult["EscapeSequences"] = QStringLiteral("So me esc\nap\te se\\qu\re\\nces");
        kpluginObj["Name"] = QStringLiteral("Example");
        kpluginObj["Name[de_DE]"] = QStringLiteral("Beispiel");
        kpluginObj["Category"] = QStringLiteral("Examples");
        kpluginObj["Dependencies"] = QJsonArray::fromStringList(QStringList() << "foo" << "bar" << "esc,aped");
        kpluginObj["ServiceTypes"] = QJsonArray::fromStringList(QStringList());
        kpluginObj["EnabledByDefault"] = true;
        kpluginObj["Version"] = QStringLiteral("1.0");
        QJsonObject compatResult = expectedResult;
        compatResult["Name"] = QStringLiteral("Example");
        compatResult["Name[de_DE]"] = QStringLiteral("Beispiel");
        compatResult["X-KDE-PluginInfo-Category"] = QStringLiteral("Examples");
        compatResult["X-KDE-PluginInfo-Version"] = QStringLiteral("1.0");
        compatResult["X-KDE-PluginInfo-Depends"] = QJsonArray::fromStringList(QStringList() << "foo" << "bar" << "esc,aped");
        compatResult["X-KDE-ServiceTypes"] = QJsonArray::fromStringList(QStringList());
        compatResult["X-KDE-PluginInfo-EnabledByDefault"] = true;

        expectedResult["KPlugin"] = kpluginObj;

        QTest::newRow("newFormat") << input << expectedResult << false;
        QTest::newRow("compatFormat") << input << compatResult << true;


        // test conversion of a currently existing .desktop file (excluding most of the translations):
        QByteArray kdevInput =
            "[Desktop Entry]\n"
            "Type=Service\n"
            "Icon=text-x-c++src\n"
            "Exec=blubb\n"
            "Comment=C/C++ Language Support\n"
            "Comment[fr]=Prise en charge du langage C/C++\n"
            "Comment[it]=Supporto al linguaggio C/C++\n"
            "Name=C++ Support\n"
            "Name[fi]=C++-tuki\n"
            "Name[fr]=Prise en charge du C++\n"
            "GenericName=Language Support\n"
            "GenericName[sl]=Podpora jeziku\n"
            "ServiceTypes=KDevelop/Plugin\n"
            "X-KDE-Library=kdevcpplanguagesupport\n"
            "X-KDE-PluginInfo-Name=kdevcppsupport\n"
            "X-KDE-PluginInfo-Category=Language Support\n"
            "X-KDevelop-Version=1\n"
            "X-KDevelop-Language=C++\n"
            "X-KDevelop-Args=CPP\n"
            "X-KDevelop-Interfaces=ILanguageSupport\n"
            "X-KDevelop-SupportedMimeTypes=text/x-chdr,text/x-c++hdr,text/x-csrc,text/x-c++src\n"
            "X-KDevelop-Mode=NoGUI\n"
            "X-KDevelop-LoadMode=AlwaysOn";

        QJsonParseError e;
        QJsonObject kdevExpected = QJsonDocument::fromJson("{\n"
            " \"GenericName\": \"Language Support\",\n"
            " \"GenericName[sl]\": \"Podpora jeziku\",\n"
            " \"KPlugin\": {\n"
            "     \"Category\": \"Language Support\",\n"
            "     \"Description\": \"C/C++ Language Support\",\n"
            "     \"Description[fr]\": \"Prise en charge du langage C/C++\",\n"
            "     \"Description[it]\": \"Supporto al linguaggio C/C++\",\n"
            "     \"Icon\": \"text-x-c++src\",\n"
            "     \"Id\": \"kdevcppsupport\",\n"
            "     \"Name\": \"C++ Support\",\n"
            "     \"Name[fi]\": \"C++-tuki\",\n"
            "     \"Name[fr]\": \"Prise en charge du C++\",\n"
            "     \"ServiceTypes\": [ \"KDevelop/Plugin\" ]\n"
            " },\n"
            " \"X-KDevelop-Args\": \"CPP\",\n"
            " \"X-KDevelop-Interfaces\": \"ILanguageSupport\",\n"
            " \"X-KDevelop-Language\": \"C++\",\n"
            " \"X-KDevelop-LoadMode\": \"AlwaysOn\",\n"
            " \"X-KDevelop-Mode\": \"NoGUI\",\n"
            " \"X-KDevelop-SupportedMimeTypes\": \"text/x-chdr,text/x-c++hdr,text/x-csrc,text/x-c++src\",\n"
            " \"X-KDevelop-Version\": \"1\"\n"
            "}\n", &e).object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        QTest::newRow("kdevcpplanguagesupport") << kdevInput << kdevExpected << false;

        // test conversion of the X-KDE-PluginInfo-Author + X-KDE-PluginInfo-Email key:
        QByteArray authorInput =
            "[Desktop Entry]\n"
            "Type=Service\n"
            "X-KDE-PluginInfo-Author=Foo Bar\n"
            "X-KDE-PluginInfo-Email=foo.bar@baz.com\n";

        QJsonObject authorsExpected = QJsonDocument::fromJson("{\n"
            " \"KPlugin\": {\n"
            "     \"Authors\": [ { \"Name\": \"Foo Bar\", \"Email\": \"foo.bar@baz.com\" } ]\n"
            " }\n }\n", &e).object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        QTest::newRow("authors") << authorInput << authorsExpected << false;


    }
    void testDesktopToJson()
    {
        QTemporaryFile output;
        QTemporaryFile inputFile;
        QVERIFY(inputFile.open());
        QVERIFY(output.open()); // create the file
        QFETCH(QByteArray, input);
        QFETCH(QJsonObject, expectedResult);
        QFETCH(bool, compatibilityMode);
        output.close();
        inputFile.write(input);
        inputFile.flush();
        inputFile.close();
        qDebug() << expectedResult;


        QProcess proc;
        proc.setProgram(DESKTOP_TO_JSON_EXE);
        QStringList arguments = QStringList() << "-i" << inputFile.fileName() << "-o" << output.fileName();
        if (compatibilityMode) {
            arguments << "-c";
        }
        proc.setArguments(arguments);
        proc.start();
        QVERIFY(proc.waitForFinished(10000));
        qDebug() << "desktoptojson STDOUT:" <<  proc.readAllStandardOutput();
        QByteArray errorOut = proc.readAllStandardError();
        if (!errorOut.isEmpty()) {
            qWarning() << "desktoptojson STDERR:" <<  errorOut;
            QFAIL("desktoptojson had errors");
        }
        QCOMPARE(proc.exitCode(), 0);
        QVERIFY(output.open());
        QByteArray jsonString = output.readAll();
        qDebug() << "result: " << jsonString;
        QJsonParseError e;
        QJsonDocument doc = QJsonDocument::fromJson(jsonString, &e);
        QCOMPARE(e.error, QJsonParseError::NoError);
        QJsonObject result = doc.object();
        compareJson(result, expectedResult);
        QVERIFY(!QTest::currentTestFailed());
    }

    void testPluginIndex()
    {
        QProcess proc;
        proc.setProgram(DESKTOP_TO_JSON_EXE);
        QStringList arguments = QStringList() << "-u" << "all";
        proc.setArguments(arguments);
        proc.start();
        QVERIFY(proc.waitForFinished(10000));
        qDebug() << "desktoptojson STDOUT:" <<  proc.readAllStandardOutput();
        qDebug() << "Plugin index.";
        QVERIFY(true);
    }
};

QTEST_MAIN(DesktopToJsonTest)

#include "desktoptojsontest.moc"
