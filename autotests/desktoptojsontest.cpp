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

private Q_SLOTS:
    void testDesktopToJson() {
        QTemporaryFile output;
        QTemporaryFile input;
        QJsonObject expectedResult;
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
        expectedResult["Name"] = "Example";
        //empty lines
        input.write("\n");
        input.write(" \n");
            // make sure translations are included:
        input.write("Name[de_DE]=Beispiel\n");
        expectedResult["Name[de_DE]"] = "Beispiel";
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
        expectedResult["X-KDE-PluginInfo-Category"] = "Examples";
        // The multiple values should be separated by a semicolon and the value of the key
        // may be optionally terminated by a semicolon. Trailing empty strings must always
        // be terminated with a semicolon. Semicolons in these values need to be escaped using \;.
        input.write("X-KDE-PluginInfo-Depends=foo,bar,esc\\,aped\n"); // string list key
        expectedResult["X-KDE-PluginInfo-Depends"] = QJsonArray::fromStringList(QStringList()
            << "foo" << "bar" << "esc,aped");
        input.write("X-KDE-ServiceTypes=\n"); // empty string list
        expectedResult["X-KDE-ServiceTypes"] = QJsonArray::fromStringList(QStringList());
        input.write("X-KDE-PluginInfo-EnabledByDefault=true\n"); // bool key
        expectedResult["X-KDE-PluginInfo-EnabledByDefault"] = true;
        // now start a new group
        input.write("[New Group]\n");
        input.write("InWrongGroup=true\n");
        input.flush();
        input.close();
        QFile f(input.fileName());
        QVERIFY(f.open(QFile::ReadOnly));
        qWarning().nospace() << f.readAll();
        f.close();
        input.setAutoRemove(false);

        QProcess proc;
        proc.setProgram(DESKTOP_TO_JSON_EXE);
        proc.setArguments(QStringList() << "-i" << input.fileName() << "-o" << output.fileName());
        proc.start();
        QVERIFY(proc.waitForFinished(10000));
        qDebug() << "STDOUT\n" <<  proc.readAllStandardOutput();
        qDebug() << "STDERR\n" <<  proc.readAllStandardError();
        QCOMPARE(proc.readAllStandardError(), QByteArray());
        QCOMPARE(proc.exitCode(), 0);
        QVERIFY(output.open());
        QByteArray jsonString = output.readAll();
        qWarning().nospace() << jsonString;
        qWarning();
        QJsonParseError e;
        QJsonDocument doc = QJsonDocument::fromJson(jsonString, &e);
        QCOMPARE(e.error, QJsonParseError::NoError);
        qWarning() << doc;
        QJsonObject result = doc.object();
        for (auto it = result.constBegin(); it != result.constEnd(); ++it) {
            qDebug() << it.key() << "=" << it.value();
            if (expectedResult.constFind(it.key()) == expectedResult.constEnd()) {
                qCritical() << "Result has key" << it.key() << "which is not expected!";
                QFAIL("Invalid output");
            }
            QCOMPARE(it.value(), expectedResult.value(it.key()));
        }
        //QCOMPARE(doc.object(), expectedResult);
    }
};

QTEST_MAIN(DesktopToJsonTest)

#include "desktoptojsontest.moc"
