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
    void testDesktopToJson() {
        QTemporaryFile output;
        QTemporaryFile input;
        QJsonObject expectedResult;
        QJsonObject kpluginObj;
        QVERIFY(input.open());
        QVERIFY(output.open()); // create the file
        output.close();
        input.write(
            // include an insignificant group
            "[Some Group]\n"
            "Foo=Bar\n"
            "\n"
        );
        input.write("[Desktop Entry]\n");
        // only data inside [Desktop Entry] should be included
        input.write("Name=Example\n");
        kpluginObj["Name"] = "Example";
        //empty lines
        input.write("\n");
        input.write(" \n");
            // make sure translations are included:
        input.write("Name[de_DE]=Beispiel\n");
        kpluginObj["Name[de_DE]"] = "Beispiel";
            // ignore comments:
        input.write("#Comment=Comment\n");
        input.write("  #Comment=Comment\n");
        input.write("Categories=foo;bar;a\\;b\n");
        expectedResult["Categories"] = "foo;bar;a\\;b";
        // As the case is significant, the keys Name and NAME are not equivalent:
        input.write("CaseSensitive=ABC\n");
        expectedResult["CaseSensitive"] = "ABC";
        input.write("CASESENSITIVE=abc\n");
        expectedResult["CASESENSITIVE"] = "abc";
        // Space before and after the equals sign should be ignored:
        input.write("SpacesBeforeEq   =foo\n");
        expectedResult["SpacesBeforeEq"] = "foo";
        input.write("SpacesAfterEq=   foo\n");
        expectedResult["SpacesAfterEq"] = "foo";
        //  Space before and after the equals sign should be ignored; the = sign is the actual delimiter.
        // TODO: error in spec (spaces before and after the key??)
        input.write("   SpacesBeforeKey=foo\n");
        expectedResult["SpacesBeforeKey"] = "foo";
        input.write("SpacesAfterKey   =foo\n");
        expectedResult["SpacesAfterKey"] = "foo";
        // ignore trailing spaces
        input.write("TrailingSpaces=foo   \n");
        expectedResult["TrailingSpaces"] = "foo";
        // However spaces in the value are significant:
        input.write("SpacesInValue=Hello, World!\n");
        expectedResult["SpacesInValue"] = "Hello, World!";
        //  The escape sequences \s, \n, \t, \r, and \\ are supported for values of
        // type string and localestring, meaning ASCII space, newline, tab,
        // carriage return, and backslash, respectively:
        input.write("EscapeSequences=So\\sme esc\\nap\\te se\\\\qu\\re\\\\nces\n"); // make sure that the last n is a literal n not a newline!
        expectedResult["EscapeSequences"] = "So me esc\nap\te se\\qu\re\\nces";
        // the standard keys that are used by plugins, make sure correct types are used:
        input.write("X-KDE-PluginInfo-Category=Examples\n"); // string key
        kpluginObj["Category"] = "Examples";
        // The multiple values should be separated by a semicolon and the value of the key
        // may be optionally terminated by a semicolon. Trailing empty strings must always
        // be terminated with a semicolon. Semicolons in these values need to be escaped using \;.
        input.write("X-KDE-PluginInfo-Depends=foo,bar,esc\\,aped\n"); // string list key
        kpluginObj["Dependencies"] = QJsonArray::fromStringList(QStringList()
            << "foo" << "bar" << "esc,aped");
        input.write("X-KDE-ServiceTypes=\n"); // empty string list
        kpluginObj["ServiceTypes"] = QJsonArray::fromStringList(QStringList());
        input.write("X-KDE-PluginInfo-EnabledByDefault=true\n"); // bool key
        kpluginObj["EnabledByDefault"] = true;
        // now start a new group
        input.write("[New Group]\n");
        input.write("InWrongGroup=true\n");
        input.flush();
        input.close();
        expectedResult["KPlugin"] = kpluginObj;


        QProcess proc;
        proc.setProgram(DESKTOP_TO_JSON_EXE);
        proc.setArguments(QStringList() << "-i" << input.fileName() << "-o" << output.fileName());
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
        //qDebug() << "result: " << jsonString;
        QJsonParseError e;
        QJsonDocument doc = QJsonDocument::fromJson(jsonString, &e);
        QCOMPARE(e.error, QJsonParseError::NoError);
        QJsonObject result = doc.object();
        compareJson(result, expectedResult);
        QVERIFY(!QTest::currentTestFailed());

    }

    // test the conversion of some existing plugin
    void testConvertKdevCppSupport()
    {
        QTemporaryFile output;
        QTemporaryFile input;
        QVERIFY(input.open());
        QVERIFY(output.open()); // create the file
        output.close();
        // this is kdevcppsupport.desktop file with a most translations stripped
        input.write(
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
            "X-KDevelop-LoadMode=AlwaysOn"
        );
        input.flush();
        input.close();

        QProcess proc;
        proc.setProgram(DESKTOP_TO_JSON_EXE);
        proc.setArguments(QStringList() << "-i" << input.fileName() << "-o" << output.fileName());
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
        //qDebug() << "result: " << jsonString;
        QJsonParseError e;
        QJsonObject result = QJsonDocument::fromJson(jsonString, &e).object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        QJsonObject expected = QJsonDocument::fromJson("{\n"
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
        compareJson(result, expected);
        QVERIFY(!QTest::currentTestFailed());
    }
};

QTEST_MAIN(DesktopToJsonTest)

#include "desktoptojsontest.moc"
