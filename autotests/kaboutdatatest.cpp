/*
 *  Copyright 2008 Friedrich W. H. Kossebau  <kossebau@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

// test object
#include <kaboutdata.h>
// Qt
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QTest>

class KAboutDataTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testLongFormConstructorWithDefaults();
    void testLongFormConstructor();
    void testShortFormConstructor();
    void testSetAddLicense();
    void testSetProgramIconName();
    void testSetDesktopFileName();
    void testCopying();

    void testKAboutDataOrganizationDomain();
};

static const char AppName[] =            "app";
static const char ProgramName[] =        "ProgramName";
static const char ProgramIconName[] =    "program-icon";
static const char Version[] =            "Version";
static const char ShortDescription[] =   "ShortDescription";
static const char CopyrightStatement[] = "CopyrightStatement";
static const char Text[] =               "Text";
static const char HomePageAddress[] =    "http://test.no.where/";
static const char HomePageSecure[] =     "https://test.no.where/";
static const char OrganizationDomain[] = "no.where";
static const char BugsEmailAddress[] =   "bugs@no.else";
static const char LicenseText[] =        "free to write, reading forbidden";
static const char LicenseFileName[] =    "testlicensefile";
static const char LicenseFileText[] =    "free to write, reading forbidden, in the file";

void KAboutDataTest::testLongFormConstructorWithDefaults()
{
    KAboutData aboutData(AppName, QLatin1String(ProgramName), Version,
                         QLatin1String(ShortDescription), KAboutLicense::Unknown);

    QCOMPARE(aboutData.componentName(), QLatin1String(AppName));
    QCOMPARE(aboutData.productName(), QLatin1String(AppName));
    QCOMPARE(aboutData.displayName(), QLatin1String(ProgramName));
    QCOMPARE(aboutData.programIconName(), QLatin1String(AppName));
    QCOMPARE(aboutData.programLogo(), QVariant());
    QCOMPARE(aboutData.organizationDomain(), QString::fromLatin1("kde.org"));
    QCOMPARE(aboutData.version(), QString::fromLatin1(Version));
    QCOMPARE(aboutData.homepage(), QString());
    QCOMPARE(aboutData.bugAddress(), QString::fromLatin1("submit@bugs.kde.org"));
    QVERIFY(aboutData.authors().isEmpty());
    QVERIFY(aboutData.credits().isEmpty());
    QVERIFY(aboutData.translators().isEmpty());
    QCOMPARE(aboutData.otherText(), QString());
    QCOMPARE(aboutData.licenses().count(), 1);
// We don't know the default text, do we?
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::ShortName).isEmpty());
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::FullName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::FullName).isEmpty());
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());
    QCOMPARE(aboutData.copyrightStatement(), QString());
    QCOMPARE(aboutData.shortDescription(), QLatin1String(ShortDescription));
    QCOMPARE(aboutData.customAuthorPlainText(), QString());
    QCOMPARE(aboutData.customAuthorRichText(), QString());
    QVERIFY(!aboutData.customAuthorTextEnabled());
    QCOMPARE(aboutData.desktopFileName(), QStringLiteral("org.kde.app"));
    //TODO: test internalVersion, internalProgramName, internalBugAddress
}

void KAboutDataTest::testLongFormConstructor()
{
    KAboutData aboutData(AppName, QLatin1String(ProgramName), Version,
                         QLatin1String(ShortDescription), KAboutLicense::Unknown,
                         QLatin1String(CopyrightStatement), QLatin1String(Text),
                         HomePageAddress, BugsEmailAddress);

    QCOMPARE(aboutData.componentName(), QLatin1String(AppName));
    QCOMPARE(aboutData.productName(), QLatin1String(AppName));
    QCOMPARE(aboutData.displayName(), QLatin1String(ProgramName));
    QCOMPARE(aboutData.programIconName(), QLatin1String(AppName));
    QCOMPARE(aboutData.programLogo(), QVariant());
    QCOMPARE(aboutData.organizationDomain(), QString::fromLatin1(OrganizationDomain));
    QCOMPARE(aboutData.version(), QString::fromLatin1(Version));
    QCOMPARE(aboutData.homepage(), QString::fromLatin1(HomePageAddress));
    QCOMPARE(aboutData.bugAddress(), QString::fromLatin1(BugsEmailAddress));
    QVERIFY(aboutData.authors().isEmpty());
    QVERIFY(aboutData.credits().isEmpty());
    QVERIFY(aboutData.translators().isEmpty());
    QCOMPARE(aboutData.otherText(), QString::fromLatin1(Text));
    QCOMPARE(aboutData.licenses().count(), 1);
// We don't know the default text, do we?
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::ShortName).isEmpty());
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::FullName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::FullName).isEmpty());
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());
    QCOMPARE(aboutData.copyrightStatement(), QLatin1String(CopyrightStatement));
    QCOMPARE(aboutData.shortDescription(), QLatin1String(ShortDescription));
    QCOMPARE(aboutData.customAuthorPlainText(), QString());
    QCOMPARE(aboutData.customAuthorRichText(), QString());
    QVERIFY(!aboutData.customAuthorTextEnabled());
    QCOMPARE(aboutData.desktopFileName(), QStringLiteral("where.no.app"));
    //TODO: test internalVersion, internalProgramName, internalBugAddress

    // We support http and https protocols on the homepage address, ensure they
    // give the same org. domain and desktop file name.
    KAboutData aboutDataSecure(AppName, QLatin1String(ProgramName), Version,
                         QLatin1String(ShortDescription), KAboutLicense::Unknown,
                         QLatin1String(CopyrightStatement), QLatin1String(Text),
                         HomePageSecure, BugsEmailAddress);
    QCOMPARE(aboutDataSecure.componentName(), QLatin1String(AppName));
    QCOMPARE(aboutDataSecure.productName(), QLatin1String(AppName));
    QCOMPARE(aboutDataSecure.organizationDomain(), QString::fromLatin1(OrganizationDomain));
    QCOMPARE(aboutDataSecure.desktopFileName(), QStringLiteral("where.no.app"));
}

void KAboutDataTest::testShortFormConstructor()
{
    KAboutData aboutData(AppName, QLatin1String(ProgramName), Version);

    QCOMPARE(aboutData.componentName(), QLatin1String(AppName));
    QCOMPARE(aboutData.productName(), QLatin1String(AppName));
    QCOMPARE(aboutData.displayName(), QLatin1String(ProgramName));
    QCOMPARE(aboutData.programIconName(), QLatin1String(AppName));
    QCOMPARE(aboutData.programLogo(), QVariant());
    QCOMPARE(aboutData.organizationDomain(), QString::fromLatin1("kde.org"));
    QCOMPARE(aboutData.version(), QString::fromLatin1(Version));
    QCOMPARE(aboutData.homepage(), QString());
    QCOMPARE(aboutData.bugAddress(), QString::fromLatin1("submit@bugs.kde.org"));
    QVERIFY(aboutData.authors().isEmpty());
    QVERIFY(aboutData.credits().isEmpty());
    QVERIFY(aboutData.translators().isEmpty());
    QCOMPARE(aboutData.otherText(), QString());
    QCOMPARE(aboutData.licenses().count(), 1);
// We don't know the default text, do we?
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::ShortName).isEmpty());
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::FullName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::FullName).isEmpty());
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());
    QCOMPARE(aboutData.copyrightStatement(), QString());
    QCOMPARE(aboutData.shortDescription(), QString());
    QCOMPARE(aboutData.customAuthorPlainText(), QString());
    QCOMPARE(aboutData.customAuthorRichText(), QString());
    QVERIFY(!aboutData.customAuthorTextEnabled());
    QCOMPARE(aboutData.desktopFileName(), QStringLiteral("org.kde.app"));
    //TODO: test internalVersion, internalProgramName, internalBugAddress
}

void KAboutDataTest::testKAboutDataOrganizationDomain()
{
    KAboutData data("app", QLatin1String("program"), "version",
                    QLatin1String("description"), KAboutLicense::LGPL,
                    QLatin1String("copyright"), QLatin1String("hello world"),
                    "http://www.koffice.org");
    QCOMPARE(data.organizationDomain(), QString::fromLatin1("koffice.org"));
    QCOMPARE(data.desktopFileName(), QStringLiteral("org.koffice.app"));

    KAboutData data2("app", QLatin1String("program"), "version",
                     QLatin1String("description"), KAboutLicense::LGPL,
                     QLatin1String("copyright"), QLatin1String("hello world"),
                     "http://edu.kde.org/kig");
    QCOMPARE(data2.organizationDomain(), QString::fromLatin1("kde.org"));
    QCOMPARE(data2.desktopFileName(), QStringLiteral("org.kde.app"));
}

void KAboutDataTest::testSetAddLicense()
{
    // prepare a file with a license text
    QFile licenseFile(QString::fromLatin1(LicenseFileName));
    licenseFile.open(QIODevice::WriteOnly);
    QTextStream licenseFileStream(&licenseFile);
    licenseFileStream << QLatin1String(LicenseFileText);
    licenseFile.close();

    const QString copyrightStatement = QLatin1String(CopyrightStatement);
    const QString lineFeed = QString::fromLatin1("\n\n");

    KAboutData aboutData(AppName, QLatin1String(ProgramName), Version,
                         QLatin1String(ShortDescription), KAboutLicense::Unknown,
                         QLatin1String(CopyrightStatement), QLatin1String(Text),
                         HomePageAddress, BugsEmailAddress);

    // set to GPL2
    aboutData.setLicense(KAboutLicense::GPL_V2);

    QCOMPARE(aboutData.licenses().count(), 1);
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString::fromLatin1("GPL v2"));
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::FullName), QString::fromLatin1("GNU General Public License Version 2"));
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(GPL2Text) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());

    // set to Unknown again
    aboutData.setLicense(KAboutLicense::Unknown);

    QCOMPARE(aboutData.licenses().count(), 1);
// We don't know the default text, do we?
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::ShortName).isEmpty());
//     QCOMPARE( aboutData.licenses().at(0).name(KAboutLicense::FullName), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).name(KAboutLicense::FullName).isEmpty());
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(WarningText) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());

    // add GPL3
    aboutData.addLicense(KAboutLicense::GPL_V3);

    QCOMPARE(aboutData.licenses().count(), 1);
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString::fromLatin1("GPL v3"));
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::FullName), QString::fromLatin1("GNU General Public License Version 3"));
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(GPL3Text) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());

    // add GPL2, Custom and File
    aboutData.addLicense(KAboutLicense::GPL_V2);
    aboutData.addLicenseText(QLatin1String(LicenseText));
    aboutData.addLicenseTextFile(QString::fromLatin1(LicenseFileName));

    QCOMPARE(aboutData.licenses().count(), 4);
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::ShortName), QString::fromLatin1("GPL v3"));
    QCOMPARE(aboutData.licenses().at(0).name(KAboutLicense::FullName), QString::fromLatin1("GNU General Public License Version 3"));
//     QCOMPARE( aboutData.licenses().at(0).text(), QString(GPL3Text) );
    QVERIFY(!aboutData.licenses().at(0).text().isEmpty());
    QCOMPARE(aboutData.licenses().at(1).name(KAboutLicense::ShortName), QString::fromLatin1("GPL v2"));
    QCOMPARE(aboutData.licenses().at(1).name(KAboutLicense::FullName), QString::fromLatin1("GNU General Public License Version 2"));
//     QCOMPARE( aboutData.licenses().at(1).text(), QString(GPL2Text) );
    QVERIFY(!aboutData.licenses().at(1).text().isEmpty());
    QCOMPARE(aboutData.licenses().at(2).name(KAboutLicense::ShortName), QString::fromLatin1("Custom"));
    QCOMPARE(aboutData.licenses().at(2).name(KAboutLicense::FullName), QString::fromLatin1("Custom"));
    QCOMPARE(aboutData.licenses().at(2).text(), QLatin1String(LicenseText));
    QCOMPARE(aboutData.licenses().at(3).name(KAboutLicense::ShortName), QString::fromLatin1("Custom"));
    QCOMPARE(aboutData.licenses().at(3).name(KAboutLicense::FullName), QString::fromLatin1("Custom"));
    QCOMPARE(aboutData.licenses().at(3).text(), QString(copyrightStatement + lineFeed + QLatin1String(LicenseFileText)));
}

void KAboutDataTest::testSetProgramIconName()
{
    const QLatin1String programIconName(ProgramIconName);

    KAboutData aboutData(AppName, QLatin1String(ProgramName), Version,
                         QLatin1String(ShortDescription), KAboutLicense::Unknown,
                         QLatin1String(CopyrightStatement), QLatin1String(Text),
                         HomePageAddress, BugsEmailAddress);

    // Deprecated, still want to test this though. Silence GCC warnings.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    // set different iconname
    aboutData.setProgramIconName(programIconName);
#pragma GCC diagnostic pop
    QCOMPARE(aboutData.programIconName(), programIconName);
}

void KAboutDataTest::testCopying()
{
    KAboutData aboutData(AppName, QLatin1String(ProgramName), Version,
                         QLatin1String(ShortDescription), KAboutLicense::GPL_V2);

    {
        KAboutData aboutData2(AppName, QLatin1String(ProgramName), Version,
                              QLatin1String(ShortDescription), KAboutLicense::GPL_V3);
        aboutData2.addLicense(KAboutLicense::GPL_V2);
        aboutData = aboutData2;
    }
    QList<KAboutLicense> licenses = aboutData.licenses();
    QCOMPARE(licenses.count(), 2);
    QCOMPARE(licenses.at(0).key(), KAboutLicense::GPL_V3);
    // check it doesn't crash
    QVERIFY(!licenses.at(0).text().isEmpty());
    QCOMPARE(licenses.at(1).key(), KAboutLicense::GPL_V2);
    // check it doesn't crash
    QVERIFY(!licenses.at(1).text().isEmpty());
}

void KAboutDataTest::testSetDesktopFileName()
{
    KAboutData aboutData(AppName, QLatin1String(ProgramName), Version,
                         QLatin1String(ShortDescription), KAboutLicense::Unknown);
    QCOMPARE(aboutData.desktopFileName(), QStringLiteral("org.kde.app"));

    // set different desktopFileName
    aboutData.setDesktopFileName(QStringLiteral("foo.bar.application"));
    QCOMPARE(aboutData.desktopFileName(), QStringLiteral("foo.bar.application"));
}

QTEST_MAIN(KAboutDataTest)

#include "kaboutdatatest.moc"
