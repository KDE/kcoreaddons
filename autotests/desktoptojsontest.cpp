/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcoreaddons_debug.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <QTemporaryFile>
#include <QTest>
#include <kcoreaddons_export.h>

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

class DesktopToJsonTest : public QObject
{
    Q_OBJECT

private:
    void compareJson(const QJsonObject &actual, const QJsonObject &expected)
    {
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
        QTest::addColumn<QStringList>("serviceTypes");

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
            // empty lines
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
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 79)
            "X-KDE-PluginInfo-Depends=foo,bar,esc\\,aped\n" // string list key
#endif
            "X-KDE-ServiceTypes=\n" // empty string list
            "X-KDE-PluginInfo-EnabledByDefault=true\n" // bool key
            // now start a new group
            "[New Group]\n"
            "InWrongGroup=true\n";

        expectedResult[QStringLiteral("Categories")] = QStringLiteral("foo;bar;a\\;b");
        expectedResult[QStringLiteral("CaseSensitive")] = QStringLiteral("ABC");
        expectedResult[QStringLiteral("CASESENSITIVE")] = QStringLiteral("abc");
        expectedResult[QStringLiteral("SpacesBeforeEq")] = QStringLiteral("foo");
        expectedResult[QStringLiteral("SpacesAfterEq")] = QStringLiteral("foo");
        expectedResult[QStringLiteral("SpacesBeforeKey")] = QStringLiteral("foo");
        expectedResult[QStringLiteral("SpacesAfterKey")] = QStringLiteral("foo");
        expectedResult[QStringLiteral("TrailingSpaces")] = QStringLiteral("foo");
        expectedResult[QStringLiteral("SpacesInValue")] = QStringLiteral("Hello, World!");
        expectedResult[QStringLiteral("EscapeSequences")] = QStringLiteral("So me esc\nap\te se\\qu\re\\nces");
        kpluginObj[QStringLiteral("Name")] = QStringLiteral("Example");
        kpluginObj[QStringLiteral("Name[de_DE]")] = QStringLiteral("Beispiel");
        kpluginObj[QStringLiteral("Category")] = QStringLiteral("Examples");
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 79)
        kpluginObj[QStringLiteral("Dependencies")] =
            QJsonArray::fromStringList(QStringList() << QStringLiteral("foo") << QStringLiteral("bar") << QStringLiteral("esc,aped"));
#endif
        kpluginObj[QStringLiteral("ServiceTypes")] = QJsonArray::fromStringList(QStringList());
        kpluginObj[QStringLiteral("EnabledByDefault")] = true;
        kpluginObj[QStringLiteral("Version")] = QStringLiteral("1.0");
        QJsonObject compatResult = expectedResult;
        compatResult[QStringLiteral("Name")] = QStringLiteral("Example");
        compatResult[QStringLiteral("Name[de_DE]")] = QStringLiteral("Beispiel");
        compatResult[QStringLiteral("X-KDE-PluginInfo-Category")] = QStringLiteral("Examples");
        compatResult[QStringLiteral("X-KDE-PluginInfo-Version")] = QStringLiteral("1.0");
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 79)
        compatResult[QStringLiteral("X-KDE-PluginInfo-Depends")] =
            QJsonArray::fromStringList(QStringList() << QStringLiteral("foo") << QStringLiteral("bar") << QStringLiteral("esc,aped"));
#endif
        compatResult[QStringLiteral("X-KDE-ServiceTypes")] = QJsonArray::fromStringList(QStringList());
        compatResult[QStringLiteral("X-KDE-PluginInfo-EnabledByDefault")] = true;

        expectedResult[QStringLiteral("KPlugin")] = kpluginObj;

        QTest::newRow("newFormat") << input << expectedResult << false << QStringList();
        QTest::newRow("compatFormat") << input << compatResult << true << QStringList();

        // test conversion of a currently existing .desktop file (excluding most of the translations):
        QByteArray kdevInput =
            "[Desktop Entry]\n"
            "Type = Service\n"
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
            "ServiceTypes=KDevelop/NonExistentPlugin\n"
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
        QJsonObject kdevExpected = QJsonDocument::fromJson(
                                       "{\n"
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
                                       "     \"ServiceTypes\": [ \"KDevelop/NonExistentPlugin\" ]\n"
                                       " },\n"
                                       " \"X-KDevelop-Args\": \"CPP\",\n"
                                       " \"X-KDevelop-Interfaces\": \"ILanguageSupport\",\n"
                                       " \"X-KDevelop-Language\": \"C++\",\n"
                                       " \"X-KDevelop-LoadMode\": \"AlwaysOn\",\n"
                                       " \"X-KDevelop-Mode\": \"NoGUI\",\n"
                                       " \"X-KDevelop-SupportedMimeTypes\": \"text/x-chdr,text/x-c++hdr,text/x-csrc,text/x-c++src\",\n"
                                       " \"X-KDevelop-Version\": \"1\"\n"
                                       "}\n",
                                       &e)
                                       .object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        QTest::newRow("kdevcpplanguagesupport no servicetype") << kdevInput << kdevExpected << false << QStringList();

        QJsonObject kdevExpectedWithServiceType =
            QJsonDocument::fromJson(
                "{\n"
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
                "     \"ServiceTypes\": [ \"KDevelop/NonExistentPlugin\" ]\n"
                " },\n"
                " \"X-KDevelop-Args\": \"CPP\",\n"
                " \"X-KDevelop-Interfaces\": [\"ILanguageSupport\"],\n"
                " \"X-KDevelop-Language\": \"C++\",\n"
                " \"X-KDevelop-LoadMode\": \"AlwaysOn\",\n"
                " \"X-KDevelop-Mode\": \"NoGUI\",\n"
                " \"X-KDevelop-SupportedMimeTypes\": [\"text/x-chdr\", \"text/x-c++hdr\", \"text/x-csrc\", \"text/x-c++src\"],\n"
                " \"X-KDevelop-Version\": 1\n"
                "}\n",
                &e)
                .object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        const QString kdevServiceTypePath = QFINDTESTDATA("data/servicetypes/fake-kdevelopplugin.desktop");
        QVERIFY(!kdevServiceTypePath.isEmpty());
        QTest::newRow("kdevcpplanguagesupport with servicetype") << kdevInput << kdevExpectedWithServiceType << false << QStringList(kdevServiceTypePath);
        // test conversion of the X-KDE-PluginInfo-Author + X-KDE-PluginInfo-Email key:
        QByteArray authorInput =
            "[Desktop Entry]\n"
            "Type=Service\n"
            "X-KDE-PluginInfo-Author=Foo Bar\n"
            "X-KDE-PluginInfo-Email=foo.bar@baz.com\n";

        QJsonObject authorsExpected = QJsonDocument::fromJson(
                                          "{\n"
                                          " \"KPlugin\": {\n"
                                          "     \"Authors\": [ { \"Name\": \"Foo Bar\", \"Email\": \"foo.bar@baz.com\" } ]\n"
                                          " }\n }\n",
                                          &e)
                                          .object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        QTest::newRow("authors") << authorInput << authorsExpected << false << QStringList();

        // test case-insensitive conversion of boolean keys
        const QString boolServiceType = QFINDTESTDATA("data/servicetypes/bool-servicetype.desktop");
        QVERIFY(!boolServiceType.isEmpty());

        QByteArray boolInput1 = "[Desktop Entry]\nType=Service\nX-Test-Bool=true\n";
        QByteArray boolInput2 = "[Desktop Entry]\nType=Service\nX-Test-Bool=TRue\n";
        QByteArray boolInput3 = "[Desktop Entry]\nType=Service\nX-Test-Bool=false\n";
        QByteArray boolInput4 = "[Desktop Entry]\nType=Service\nX-Test-Bool=FALse\n";

        auto boolResultTrue = QJsonDocument::fromJson("{\"KPlugin\":{},\"X-Test-Bool\": true}", &e).object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        auto boolResultFalse = QJsonDocument::fromJson("{\"KPlugin\":{},\"X-Test-Bool\": false}", &e).object();
        QCOMPARE(e.error, QJsonParseError::NoError);
        QTest::newRow("bool true") << boolInput1 << boolResultTrue << false << QStringList(boolServiceType);
        QTest::newRow("bool TRue") << boolInput2 << boolResultTrue << false << QStringList(boolServiceType);
        QTest::newRow("bool false") << boolInput3 << boolResultFalse << false << QStringList(boolServiceType);
        QTest::newRow("bool FALse") << boolInput4 << boolResultFalse << false << QStringList(boolServiceType);

        // test conversion of kcookiejar.desktop (for some reason the wrong boolean values were committed)
        QByteArray kcookiejarInput =
            "[Desktop Entry]\n"
            "Type= Service\n"
            "Name=Cookie Jar\n"
            "Comment=Stores network cookies\n"
            "X-KDE-ServiceTypes=KDEDModule\n"
            "X-KDE-Library=kf5/kded/kcookiejar\n"
            "X-KDE-Kded-autoload=false\n"
            "X-KDE-Kded-load-on-demand=true\n";
        auto kcookiejarResult = QJsonDocument::fromJson(
                                    "{\n"
                                    "  \"KPlugin\": {\n"
                                    "    \"Description\": \"Stores network cookies\",\n"
                                    "    \"Name\": \"Cookie Jar\",\n"
                                    "    \"ServiceTypes\": [\n"
                                    "      \"KDEDModule\"\n"
                                    "    ]\n"
                                    "  },\n"
                                    "\"X-KDE-Kded-autoload\": false,\n"
                                    "\"X-KDE-Kded-load-on-demand\": true\n"
                                    "}\n",
                                    &e)
                                    .object();
        const QString kdedmoduleServiceType = QFINDTESTDATA("data/servicetypes/fake-kdedmodule.desktop");
        QVERIFY(!kdedmoduleServiceType.isEmpty());
        QTest::newRow("kcookiejar") << kcookiejarInput << kcookiejarResult << false << QStringList(kdedmoduleServiceType);
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
        QFETCH(QStringList, serviceTypes);
        output.close();
        inputFile.write(input);
        inputFile.flush();
        inputFile.close();

        QProcess proc;
        proc.setProgram(QStringLiteral(DESKTOP_TO_JSON_EXE));
        QStringList arguments = QStringList() << QStringLiteral("-i") << inputFile.fileName() << QStringLiteral("-o") << output.fileName();
        if (compatibilityMode) {
            arguments << QStringLiteral("-c");
        }
        for (const QString &s : std::as_const(serviceTypes)) {
            arguments << QStringLiteral("-s") << s;
        }
        proc.setArguments(arguments);
        proc.start();
        QVERIFY(proc.waitForFinished(10000));
        QByteArray errorOut = proc.readAllStandardError();
        if (!errorOut.isEmpty()) {
            qCWarning(KCOREADDONS_DEBUG).nospace() << "desktoptojson STDERR:\n\n" << errorOut.constData() << "\n";
        }
        QCOMPARE(proc.exitCode(), 0);
        QVERIFY(output.open());
        QByteArray jsonString = output.readAll();
        QJsonParseError e;
        QJsonDocument doc = QJsonDocument::fromJson(jsonString, &e);
        QCOMPARE(e.error, QJsonParseError::NoError);
        QJsonObject result = doc.object();
        compareJson(result, expectedResult);
        QVERIFY(!QTest::currentTestFailed());
    }
};

QTEST_MAIN(DesktopToJsonTest)

#include "desktoptojsontest.moc"
