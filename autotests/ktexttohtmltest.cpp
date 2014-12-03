/*
  Copyright (C) 2005 Ingo Kloecker <kloecker@kde.org>
  Copyright (C) 2007 Allen Winter <winter@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "ktexttohtmltest.h"

#include "../src/lib/text/ktexttohtml.h"
#include "../src/lib/text/ktexttohtml_p.h"

#include <QtTest>
#include <QDebug>
#include <QUrl>

QTEST_MAIN(KTextToHTMLTest)

Q_DECLARE_METATYPE(KTextToHTML::Options)

void KTextToHTMLTest::testGetEmailAddress()
{
    // empty input
    const QString emptyQString;
    KTextToHTMLHelper ll1(emptyQString, 0);
    QVERIFY(ll1.getEmailAddress().isEmpty());

    // no '@' at scan position
    KTextToHTMLHelper ll2(QLatin1String("foo@bar.baz"), 0);
    QVERIFY(ll2.getEmailAddress().isEmpty());

    // '@' in local part
    KTextToHTMLHelper ll3(QLatin1String("foo@bar@bar.baz"), 7);
    QVERIFY(ll3.getEmailAddress().isEmpty());

    // empty local part
    KTextToHTMLHelper ll4(QLatin1String("@bar.baz"), 0);
    QVERIFY(ll4.getEmailAddress().isEmpty());
    KTextToHTMLHelper ll5(QLatin1String(".@bar.baz"), 1);
    QVERIFY(ll5.getEmailAddress().isEmpty());
    KTextToHTMLHelper ll6(QLatin1String(" @bar.baz"), 1);
    QVERIFY(ll6.getEmailAddress().isEmpty());
    KTextToHTMLHelper ll7(QLatin1String(".!#$%&'*+-/=?^_`{|}~@bar.baz"),
                    strlen(".!#$%&'*+-/=?^_`{|}~"));
    QVERIFY(ll7.getEmailAddress().isEmpty());

    // allowed special chars in local part of address
    KTextToHTMLHelper ll8(QLatin1String("a.!#$%&'*+-/=?^_`{|}~@bar.baz"),
                    strlen("a.!#$%&'*+-/=?^_`{|}~"));
    QCOMPARE(ll8.getEmailAddress(), QLatin1String("a.!#$%&'*+-/=?^_`{|}~@bar.baz"));

    // '@' in domain part
    KTextToHTMLHelper ll9(QLatin1String("foo@bar@bar.baz"), 3);
    QVERIFY(ll9.getEmailAddress().isEmpty());

    // domain part without dot
    KTextToHTMLHelper lla(QLatin1String("foo@bar"), 3);
    QVERIFY(lla.getEmailAddress().isEmpty());
    KTextToHTMLHelper llb(QLatin1String("foo@bar."), 3);
    QVERIFY(llb.getEmailAddress().isEmpty());
    KTextToHTMLHelper llc(QLatin1String(".foo@bar"), 4);
    QVERIFY(llc.getEmailAddress().isEmpty());
    KTextToHTMLHelper lld(QLatin1String("foo@bar "), 3);
    QVERIFY(lld.getEmailAddress().isEmpty());
    KTextToHTMLHelper lle(QLatin1String(" foo@bar"), 4);
    QVERIFY(lle.getEmailAddress().isEmpty());
    KTextToHTMLHelper llf(QLatin1String("foo@bar-bar"), 3);
    QVERIFY(llf.getEmailAddress().isEmpty());

    // empty domain part
    KTextToHTMLHelper llg(QLatin1String("foo@"), 3);
    QVERIFY(llg.getEmailAddress().isEmpty());
    KTextToHTMLHelper llh(QLatin1String("foo@."), 3);
    QVERIFY(llh.getEmailAddress().isEmpty());
    KTextToHTMLHelper lli(QLatin1String("foo@-"), 3);
    QVERIFY(lli.getEmailAddress().isEmpty());

    // simple address
    KTextToHTMLHelper llj(QLatin1String("foo@bar.baz"), 3);
    QCOMPARE(llj.getEmailAddress(), QLatin1String("foo@bar.baz"));
    KTextToHTMLHelper llk(QLatin1String("foo@bar.baz."), 3);
    QCOMPARE(llk.getEmailAddress(), QLatin1String("foo@bar.baz"));
    KTextToHTMLHelper lll(QLatin1String(".foo@bar.baz"), 4);
    QCOMPARE(lll.getEmailAddress(), QLatin1String("foo@bar.baz"));
    KTextToHTMLHelper llm(QLatin1String("foo@bar.baz-"), 3);
    QCOMPARE(llm.getEmailAddress(), QLatin1String("foo@bar.baz"));
    KTextToHTMLHelper lln(QLatin1String("-foo@bar.baz"), 4);
    QCOMPARE(lln.getEmailAddress(), QLatin1String("foo@bar.baz"));
    KTextToHTMLHelper llo(QLatin1String("foo@bar.baz "), 3);
    QCOMPARE(llo.getEmailAddress(), QLatin1String("foo@bar.baz"));
    KTextToHTMLHelper llp(QLatin1String(" foo@bar.baz"), 4);
    QCOMPARE(llp.getEmailAddress(), QLatin1String("foo@bar.baz"));
    KTextToHTMLHelper llq(QLatin1String("foo@bar-bar.baz"), 3);
    QCOMPARE(llq.getEmailAddress(), QLatin1String("foo@bar-bar.baz"));
}

void KTextToHTMLTest::testGetUrl()
{
    QStringList brackets;
    brackets << QLatin1String("") << QLatin1String("");   // no brackets
    brackets << QLatin1String("<") << QLatin1String(">");
    brackets << QLatin1String("[") << QLatin1String("]");
    brackets << QLatin1String("\"") << QLatin1String("\"");
    brackets << QLatin1String("<link>") << QLatin1String("</link>");

    for (int i = 0; i < brackets.count(); i += 2) {
        testGetUrl2(brackets[ i ], brackets[ i + 1 ]);
    }
}

void KTextToHTMLTest::testGetUrl2(const QString &left, const QString &right)
{
    QStringList schemas;
    schemas << QLatin1String("http://");
    schemas << QLatin1String("https://");
    schemas << QLatin1String("vnc://");
    schemas << QLatin1String("fish://");
    schemas << QLatin1String("ftp://");
    schemas << QLatin1String("ftps://");
    schemas << QLatin1String("sftp://");
    schemas << QLatin1String("smb://");
    schemas << QLatin1String("file://");

    QStringList urls;
    urls << QLatin1String("www.kde.org");
    urls << QLatin1String("user@www.kde.org");
    urls << QLatin1String("user:pass@www.kde.org");
    urls << QLatin1String("user:pass@www.kde.org:1234");
    urls << QLatin1String("user:pass@www.kde.org:1234/sub/path");
    urls << QLatin1String("user:pass@www.kde.org:1234/sub/path?a=1");
    urls << QLatin1String("user:pass@www.kde.org:1234/sub/path?a=1#anchor");
    urls << QLatin1String("user:pass@www.kde.org:1234/sub/\npath  \n /long/  path \t  ?a=1#anchor");
    urls << QLatin1String("user:pass@www.kde.org:1234/sub/path/special(123)?a=1#anchor");
    urls << QLatin1String("user:pass@www.kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor");
    urls << QLatin1String("user:pass@www.kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla");
    urls << QLatin1String("user:pass@www.kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla]");
    urls << QLatin1String("user:pass@www.kde.org:1234/\nsub/path:with:colon/\nspecial(123)?\na=1#anchor[bla]");
    urls << QLatin1String("user:pass@www.kde.org:1234/  \n  sub/path:with:colon/  \n\t   \t   special(123)?"
                          "\n\t  \n\t   a=1#anchor[bla]");

    foreach (const QString &schema, schemas) {
        foreach (QString url, urls) {   //krazy:exclude=foreach
            // by definition: if the URL is enclosed in brackets, the URL itself is not allowed
            // to contain the closing bracket, as this would be detected as the end of the URL
            if ((left.length() == 1) && (url.contains(right[ 0 ]))) {
                continue;
            }

            // if the url contains a whitespace, it must be enclosed with brackets
            if ((url.contains(QLatin1Char('\n')) || url.contains(QLatin1Char('\t')) || url.contains(QLatin1Char(' '))) &&
                    left.isEmpty()) {
                continue;
            }

            QString test(left + schema + url + right);
            KTextToHTMLHelper ll(test, left.length());
            QString gotUrl = ll.getUrl();

            // we want to have the url without whitespace
            url.remove(QLatin1Char(' '));
            url.remove(QLatin1Char('\n'));
            url.remove(QLatin1Char('\t'));

            bool ok = (gotUrl == (schema + url));
            //qDebug() << "check:" << (ok ? "OK" : "NOK") << test << "=>" << (schema + url);
            if (!ok) {
                qDebug() << "got:" << gotUrl;
            }
            QVERIFY2(ok, qPrintable(test));
        }
    }

    QStringList urlsWithoutSchema;
    urlsWithoutSchema << QLatin1String(".kde.org");
    urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path");
    urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path?a=1");
    urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path?a=1#anchor");
    urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path/special(123)?a=1#anchor");
    urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor");
    urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla");
    urlsWithoutSchema << QLatin1String(".kde.org:1234/sub/path:with:colon/special(123)?a=1#anchor[bla]");
    urlsWithoutSchema << QLatin1String(".kde.org:1234/\nsub/path:with:colon/\nspecial(123)?\na=1#anchor[bla]");
    urlsWithoutSchema << QLatin1String(".kde.org:1234/  \n  sub/path:with:colon/  \n\t   \t   special(123)?"
                                       "\n\t  \n\t   a=1#anchor[bla]");

    QStringList starts;
    starts << QLatin1String("www") << QLatin1String("ftp") << QLatin1String("news:www");

    foreach (const QString &start, starts) {
        foreach (QString url, urlsWithoutSchema) {   //krazy:exclude=foreach
            // by definition: if the URL is enclosed in brackets, the URL itself is not allowed
            // to contain the closing bracket, as this would be detected as the end of the URL
            if ((left.length() == 1) && (url.contains(right[ 0 ]))) {
                continue;
            }

            // if the url contains a whitespace, it must be enclosed with brackets
            if ((url.contains(QLatin1Char('\n')) || url.contains(QLatin1Char('\t')) || url.contains(QLatin1Char(' '))) &&
                    left.isEmpty()) {
                continue;
            }

            QString test(left + start + url + right);
            KTextToHTMLHelper ll(test, left.length());
            QString gotUrl = ll.getUrl();

            // we want to have the url without whitespace
            url.remove(QLatin1Char(' '));
            url.remove(QLatin1Char('\n'));
            url.remove(QLatin1Char('\t'));

            bool ok = (gotUrl == (start + url));
            //qDebug() << "check:" << (ok ? "OK" : "NOK") << test << "=>" << (start + url);
            if (!ok) {
                qDebug() << "got:" << gotUrl;
            }
            QVERIFY2(ok, qPrintable(gotUrl));
        }
    }

    // test max url length
    QString url = QLatin1String("http://www.kde.org/this/is/a_very_loooooong_url/test/test/test");
    {
        KTextToHTMLHelper ll(url, 0, 10);
        QVERIFY(ll.getUrl().isEmpty());    // url too long
    }
    {
        KTextToHTMLHelper ll(url, 0, url.length() - 1);
        QVERIFY(ll.getUrl().isEmpty());    // url too long
    }
    {
        KTextToHTMLHelper ll(url, 0, url.length());
        QCOMPARE(ll.getUrl(), url);
    }
    {
        KTextToHTMLHelper ll(url, 0, url.length() + 1);
        QCOMPARE(ll.getUrl(), url);
    }

    // mailto
    {
        QString addr = QLatin1String("mailto:test@kde.org");
        QString test(left + addr + right);
        KTextToHTMLHelper ll(test, left.length());

        QString gotUrl = ll.getUrl();

        bool ok = (gotUrl == addr);
        //qDebug() << "check:" << (ok ? "OK" : "NOK") << test << "=>" << addr;
        if (!ok) {
            qDebug() << "got:" << gotUrl;
        }
        QVERIFY2(ok, qPrintable(gotUrl));
    }
}

void KTextToHTMLTest::testHtmlConvert_data()
{
    QTest::addColumn<QString>("plainText");
    QTest::addColumn<KTextToHTML::Options>("flags");
    QTest::addColumn<QString>("htmlText");

    // Linker error when using PreserveSpaces, therefore the hardcoded 0x01 or 0x09

    // Test preserving whitespace correctly
    QTest::newRow("") << " foo"
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                      << "&nbsp;foo";
    QTest::newRow("") << "  foo"
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                      << "&nbsp;&nbsp;foo";
    QTest::newRow("") << "  foo  "
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                      << "&nbsp;&nbsp;foo&nbsp;&nbsp;";
    QTest::newRow("") << "  foo "
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                      << "&nbsp;&nbsp;foo&nbsp;";
    QTest::newRow("") << "bla bla bla bla bla"
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                      << "bla bla bla bla bla";
    QTest::newRow("") << "bla bla bla \n  bla bla bla "
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                      << "bla bla bla&nbsp;<br />\n&nbsp;&nbsp;bla bla bla&nbsp;";
    QTest::newRow("") << "bla bla  bla"
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                      << "bla bla&nbsp;&nbsp;bla";
    QTest::newRow("") << " bla bla \n bla bla a\n  bla bla "
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                      << "&nbsp;bla bla&nbsp;<br />\n&nbsp;bla bla a<br />\n"
                         "&nbsp;&nbsp;bla bla&nbsp;";

    // Test highlighting with *, / and _
    QTest::newRow("") << "Ce paragraphe _contient_ des mots ou des _groupes de mots_ à mettre en"
                         " forme…"
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                      << "Ce paragraphe <u>_contient_</u> des mots ou des"
                         " <u>_groupes de mots_</u> à mettre en forme…";
    QTest::newRow("punctation-bug") << "Ce texte *a l'air* de _fonctionner_, à condition"
                                       " d’utiliser le guillemet ASCII."
                                    << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                    << "Ce texte <b>*a l'air*</b> de <u>_fonctionner_</u>, à"
                                       " condition d’utiliser le guillemet ASCII.";
    QTest::newRow("punctation-bug") << "Un répertoire /est/ un *dossier* où on peut mettre des"
                                       " *fichiers*."
                                    << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                    << "Un répertoire <i>/est/</i> un"
                                       " <b>*dossier*</b> où on peut mettre des <b>*fichiers*</b>.";
    QTest::newRow("punctation-bug") << "*BLA BLA BLA BLA*."
                                    << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                                    << "<b>BLA BLA BLA BLA</b>.";
    QTest::newRow("") << "Je vais tenter de repérer des faux positif*"
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                      << "Je vais tenter de repérer des faux positif*";
    QTest::newRow("") << "*Ouais !* *Yes!*"
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                      << "<b>*Ouais !*</b> <b>*Yes!*</b>";
    QTest::newRow("") << "the /etc/{rsyslog.d,syslog-ng.d}/package.rpmnew file"
                      << KTextToHTML::Options(KTextToHTML::PreserveSpaces | KTextToHTML::HighlightText)
                      << "the /etc/{rsyslog.d,syslog-ng.d}/package.rpmnew file";

    // This test has problems with the encoding, apparently.
    //QTest::newRow( "" ) << "*Ça fait plaisir de pouvoir utiliser des lettres accentuées dans du"
    //                       " texte mis en forme*." << 0x09 << "<b>Ça fait plaisir de pouvoir"
    //                       " utiliser des lettres accentuées dans du texte mis en forme</b>.";

    // Bug reported by dfaure, the <hostname> would get lost
    QTest::newRow("") << "QUrl url(\"http://strange<hostname>/\");"
                      << KTextToHTML::Options(KTextToHTML::ReplaceSmileys | KTextToHTML::HighlightText)
                      << "QUrl url(&quot;<a href=\"http://strange<hostname>/\">"
                         "http://strange&lt;hostname&gt;/</a>&quot;);";

    // Bug: 211128 - plain text emails should not replace ampersand & with &amp;
    QTest::newRow("bug211128") << "https://green-site/?Ticket=85&Page=next"
                               << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                               << "<a href=\"https://green-site/?Ticket=85&Page=next\">"
                                  "https://green-site/?Ticket=85&amp;Page=next</a>";

    QTest::newRow("dotBeforeEnd") << "Look at this file: www.example.com/example.h"
                                  << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                  << "Look at this file: <a href=\"http://www.example.com/example.h\">"
                                     "www.example.com/example.h</a>";
    QTest::newRow("dotInMiddle") << "Look at this file: www.example.com/.bashrc"
                                 << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                 << "Look at this file: <a href=\"http://www.example.com/.bashrc\">"
                                    "www.example.com/.bashrc</a>";

    // A dot at the end of an URL is explicitly ignored
    QTest::newRow("dotAtEnd") << "Look at this file: www.example.com/test.cpp."
                              << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                              << "Look at this file: <a href=\"http://www.example.com/test.cpp\">"
                                 "www.example.com/test.cpp</a>.";

    // Bug 313719 - URL in parenthesis
    QTest::newRow("url-in-parenthesis-1") << "KDE (website http://www.kde.org)"
                                          << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                          << "KDE (website <a href=\"http://www.kde.org\">http://www.kde.org</a>)";
    QTest::newRow("url-in-parenthesis-2") << "KDE website (http://www.kde.org)"
                                          << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                          << "KDE website (<a href=\"http://www.kde.org\">http://www.kde.org</a>)";
    QTest::newRow("url-in-parenthesis-3") << "bla (http://www.kde.org - section 5.2)"
                                          << KTextToHTML::Options(KTextToHTML::PreserveSpaces)
                                          << "bla (<a href=\"http://www.kde.org\">http://www.kde.org</a> - section 5.2)";
}


void KTextToHTMLTest::testHtmlConvert()
{
    QFETCH(QString, plainText);
    QFETCH(KTextToHTML::Options, flags);
    QFETCH(QString, htmlText);

    QEXPECT_FAIL("punctation-bug", "Linklocator does not properly detect punctation as boundaries",
                 Continue);

    QString actualHtml = KTextToHTML::convertToHtml(plainText, flags);
    QCOMPARE(actualHtml, htmlText);
}
