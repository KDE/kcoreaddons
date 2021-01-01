
#include "kstringhandlertest.h"

#include <QRegularExpression>
#include <QTest>

QTEST_MAIN(KStringHandlerTest)

#include "kstringhandler.h"

QString KStringHandlerTest::test = QStringLiteral("The quick brown fox jumped over the lazy bridge. ");

void KStringHandlerTest::capwords()
{
    QCOMPARE(KStringHandler::capwords(test),
             QStringLiteral("The Quick Brown Fox Jumped Over The Lazy Bridge. "));
}

void KStringHandlerTest::tagURLs()
{
    QString test = QStringLiteral("Click on https://foo@bar:www.kde.org/yoyo/dyne.html#a1 for info.");
    QCOMPARE(KStringHandler::tagUrls(test),
             QStringLiteral("Click on <a href=\"https://foo@bar:www.kde.org/yoyo/dyne.html#a1\">https://foo@bar:www.kde.org/yoyo/dyne.html#a1</a> for info."));

    test = QStringLiteral("http://www.foo.org/story$806");
    QCOMPARE(KStringHandler::tagUrls(test),
             QStringLiteral("<a href=\"http://www.foo.org/story$806\">http://www.foo.org/story$806</a>"));

#if 0
    // XFAIL - i.e. this needs to be fixed, but has never been
    test = "&lt;a href=www.foo.com&gt;";
    check("tagURLs()", KStringHandler::tagURLs(test),
          "&lt;a href=<a href=\"www.foo.com\">www.foo.com</a>&gt;");
#endif

    test = QStringLiteral("http://www.foo.org/bla-(bli)");
    QCOMPARE(KStringHandler::tagUrls(test),
             QStringLiteral("<a href=\"http://www.foo.org/bla-(bli)\">http://www.foo.org/bla-(bli)</a>"));

    test = QStringLiteral("http://www.foo.org/bla-bli");
    QCOMPARE(KStringHandler::tagUrls(test),
             QStringLiteral("<a href=\"http://www.foo.org/bla-bli\">http://www.foo.org/bla-bli</a>"));
}

void KStringHandlerTest::perlSplit()
{
    QStringList expected;
    expected << QStringLiteral("some") << QStringLiteral("string") << QStringLiteral("for")
             << QStringLiteral("you__here");
    QCOMPARE(KStringHandler::perlSplit(QStringLiteral("__"), QStringLiteral("some__string__for__you__here"), 4), expected);

    expected.clear();
    expected << QStringLiteral("kparts") << QStringLiteral("reaches") << QStringLiteral("the parts other parts can't");
    QCOMPARE(KStringHandler::perlSplit(QLatin1Char(' '),
                                       QStringLiteral("kparts reaches the parts other parts can't"), 3), expected);

    expected.clear();
    expected << QStringLiteral("Split") << QStringLiteral("me") << QStringLiteral("up ! I'm bored ! OK ?");
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 67)
    QCOMPARE(KStringHandler::perlSplit(QRegExp(QStringLiteral("[! ]")),
                                       QStringLiteral("Split me up ! I'm bored ! OK ?"), 3), expected);
#endif
    QCOMPARE(KStringHandler::perlSplit(QRegularExpression(QStringLiteral("[! ]")),
                                       QStringLiteral("Split me up ! I'm bored ! OK ?"), 3), expected);
}

void KStringHandlerTest::obscure()
{
    // See bug 167900, obscure() produced chars that could not properly be converted to and from
    // UTF8. The result was that storing passwords with '!' in them did not work.
    QString test = QStringLiteral("!TEST!");
    QString obscured = KStringHandler::obscure(test);
    QByteArray obscuredBytes = obscured.toUtf8();
    QCOMPARE(KStringHandler::obscure(QString::fromUtf8(obscuredBytes.constData())), test);
}

void KStringHandlerTest::preProcessWrap_data()
{
    const QChar zwsp(0x200b);

    QTest::addColumn<QString>("string");
    QTest::addColumn<QString>("expected");

    // Should result in no additional breaks
    QTest::newRow("spaces") << "foo bar baz" << "foo bar baz";

    // Should insert a ZWSP after each '_'
    QTest::newRow("underscores") << "foo_bar_baz"
                                 << QString(QStringLiteral("foo_") + zwsp + QStringLiteral("bar_") + zwsp + QStringLiteral("baz"));

    // Should insert a ZWSP after each '-'
    QTest::newRow("hyphens") << "foo-bar-baz"
                             << QString(QStringLiteral("foo-") + zwsp +
                                        QStringLiteral("bar-") + zwsp + QStringLiteral("baz"));

    // Should insert a ZWSP after each '.'
    QTest::newRow("periods") << "foo.bar.baz"
                             << QString(QStringLiteral("foo.") +
                                        zwsp + QStringLiteral("bar.") + zwsp + QStringLiteral("baz"));

    // Should insert a ZWSP after each ','
    QTest::newRow("commas") << "foo,bar,baz"
                            << QString(QStringLiteral("foo,") + zwsp +
                                       QStringLiteral("bar,") + zwsp + QStringLiteral("baz"));

    // Should result in no additional breaks since the '_'s are followed by spaces
    QTest::newRow("mixed underscores and spaces")
            << "foo_ bar_ baz" << "foo_ bar_ baz";

    // Should result in no additional breaks since the '_' is the last char
    QTest::newRow("ends with underscore") << "foo_" << "foo_";

    // Should insert a ZWSP before '(' and after ')'
    QTest::newRow("parens") << "foo(bar)baz"
                            << QString(QStringLiteral("foo") + zwsp +
                                       QStringLiteral("(bar)") + zwsp + QStringLiteral("baz"));

    // Should insert a ZWSP before '[' and after ']'
    QTest::newRow("brackets") << "foo[bar]baz"
                              << QString(QStringLiteral("foo") + zwsp +
                                         QStringLiteral("[bar]") + zwsp + QStringLiteral("baz"));

    // Should insert a ZWSP before '{' and after '}'
    QTest::newRow("curly braces") << "foo{bar}baz"
                                  << QString(QStringLiteral("foo") + zwsp +
                                             QStringLiteral("{bar}") + zwsp + QStringLiteral("baz"));

    // Should insert a ZWSP before '(' but not after ')' since it's the last char
    QTest::newRow("ends with ')'") << "foo(bar)"
                                   << QString(QStringLiteral("foo") + zwsp + QStringLiteral("(bar)"));

    // Should insert a single ZWSP between the '_' and the '('
    QTest::newRow("'_' followed by '('") << "foo_(bar)"
                                         << QString(QStringLiteral("foo_") + zwsp + QStringLiteral("(bar)"));

    // Should insert ZWSP's between the '_' and the '[', between the double
    // '['s and the double ']'s, but not before and after 'bar'
    QTest::newRow("'_' before double brackets") << "foo_[[bar]]"
            << QString(QStringLiteral("foo_") + zwsp +
                       QStringLiteral("[") + zwsp + QStringLiteral("[bar]") + zwsp +
                       QStringLiteral("]"));

    // Should only insert ZWSP's between the double '['s and the double ']'s
    QTest::newRow("space before double brackets") << "foo [[bar]]"
            << QString(QStringLiteral("foo [") + zwsp + QStringLiteral("[bar]") + zwsp + QStringLiteral("]"));

    // Shouldn't result in any additional breaks since the '(' is preceded
    // by a space, and the ')' is followed by a space.
    QTest::newRow("parens with spaces") << "foo (bar) baz" << "foo (bar) baz";

    // Should insert a WJ (Word Joiner) before a single quote
    const QChar wj(0x2060);
    QTest::newRow("single quote") << "foo'bar" << QString(QStringLiteral("foo") + QString(wj) + QStringLiteral("'bar"));
}

static QString replaceZwsp(const QString &string)
{
    const QChar zwsp(0x200b);

    QString result;
    for (int i = 0; i < string.length(); i++)
        if (string[i] == zwsp) {
            result += QStringLiteral("<zwsp>");
        } else {
            result += string[i];
        }

    return result;
}

void KStringHandlerTest::preProcessWrap()
{
    QFETCH(QString, string);
    QFETCH(QString, expected);

    QCOMPARE(replaceZwsp(KStringHandler::preProcessWrap(string)),
             replaceZwsp(expected));
}

void KStringHandlerTest::logicalLength_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<int>("expected");

    QTest::newRow("Latin") << "foo bar baz" << 11;
    QTest::newRow("Chinese") << QString::fromUtf8("\xe4\xbd\xa0\xe5\xa5\xbd") << 4;
    QTest::newRow("Japanese") << QString::fromUtf8("\xe9\x9d\x92\xe3\x81\x84\xe7\xa9\xba") << 6;
    QTest::newRow("Korean") << QString::fromUtf8("\xed\x95\x9c\xea\xb5\xad\xec\x96\xb4") << 6;
    QTest::newRow("Mixed") << QString::fromUtf8("KDE\xe6\xa1\x8c\xe9\x9d\xa2") << 7;
}

void KStringHandlerTest::logicalLength()
{
    QFETCH(QString, string);
    QFETCH(int, expected);
    QCOMPARE(KStringHandler::logicalLength(string), expected);
}

void KStringHandlerTest::subsequence_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("text");
    QTest::addColumn<bool>("expectedCaseSensitive");
    QTest::addColumn<bool>("expectedCaseInsensitive");

    QTest::newRow("empty pattern") << "" << "not empty" << false << false;
    QTest::newRow("empty text") << "pattern" << "" << false << false;

    QTest::newRow("two words inside") << "fox dog" << "The quick brown fox jumps over the lazy dog" << true << true;
    QTest::newRow("old acronym") << "kde" << "Kool Desktop Environment" << false << true;
    QTest::newRow("new acronym") << "kde" << "K Desktop Environment" << false << true;
    QTest::newRow("capitalized acronym") << "KDE" << "K Desktop Environment" << true << true;

    QTest::newRow("inside a word") << "libremath" << "LibreOffice Math" << false << true;
    QTest::newRow("pattern with space") << "Libre Math" << "LibreOffice Math" << true << true;

    QTest::newRow("no match") << "match" << "no milk" << false << false;
    QTest::newRow("match at end") << "end" << "at the end" << true << true;
    QTest::newRow("beyond the end") << "end" << "at the en" << false << false;
}

void KStringHandlerTest::subsequence()
{
    QFETCH(QString, pattern);
    QFETCH(QString, text);
    QFETCH(bool, expectedCaseSensitive);
    QFETCH(bool, expectedCaseInsensitive);

    QCOMPARE(KStringHandler::isSubsequence(pattern, text, Qt::CaseSensitive), expectedCaseSensitive);
    QCOMPARE(KStringHandler::isSubsequence(pattern, text, Qt::CaseInsensitive), expectedCaseInsensitive);
}
