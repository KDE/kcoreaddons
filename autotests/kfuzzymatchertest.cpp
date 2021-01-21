#include "kfuzzymatchertest.h"

#include <QString>
#include <QStringList>
#include <QTest>
#include <algorithm>

QTEST_MAIN(KFuzzyMatcherTest)

#include "kfuzzymatcher.h"

void KFuzzyMatcherTest::testMatchSimple()
{
    QCOMPARE(KFuzzyMatcher::matchSimple(QStringLiteral("AbcD"), QStringLiteral("AbCdefg")), true);
    QCOMPARE(KFuzzyMatcher::matchSimple(QStringLiteral("Wa qa"), QStringLiteral("Wa qar")), true);
    QCOMPARE(KFuzzyMatcher::matchSimple(QStringLiteral("ارو"), QStringLiteral("اردو")), true);
    QCOMPARE(KFuzzyMatcher::matchSimple(QStringLiteral("tf"), QStringLiteral("the_file")), true);
    QCOMPARE(KFuzzyMatcher::matchSimple(QStringLiteral("Häu"), QStringLiteral("Häuser")), true);
    QCOMPARE(KFuzzyMatcher::matchSimple(QStringLiteral("Name"), QStringLiteral("Nam")), false);
}

void KFuzzyMatcherTest::testMatch_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<int>("size");

    QTest::newRow("pattern=sort") << QStringLiteral("sort")
                          << QStringList{
                                QStringLiteral("Sort"),
                                QStringLiteral("Some other right test"),
                                QStringLiteral("Soup rate"),
                                QStringLiteral("Someother"),
                                QStringLiteral("irrelevant"),
                              }
                          << QStringList{
                                QStringLiteral("Some other right test"),
                                QStringLiteral("Sort"),
                                QStringLiteral("Soup rate"),
                              }
                          << 3;


    QTest::newRow("pattern=kateapp") << QStringLiteral("kaapp")
                          << QStringList{
                                QStringLiteral("kateapp.cpp"),
                                QStringLiteral("kate_application"),
                                QStringLiteral("kateapp.h"),
                                QStringLiteral("katepap.c")
                              }
                          << QStringList{
                                QStringLiteral("kate_application"),
                                QStringLiteral("kateapp.h"),
                                QStringLiteral("kateapp.cpp")
                             }
                          << 3;
}

template <bool (*MatchFunc)(const QStringView, const QStringView, int &)>
static QStringList matchHelper(const QString& pattern, const QStringList& input)
{
    QVector<QPair<QString, int>> actual;
    for (int i = 0; i < input.size(); ++i) {
        int score = 0;
        bool res = MatchFunc(pattern, input.at(i), score);
        if (res) {
            actual.push_back({input.at(i), score});
        }
    }

    // sort descending based on score
    std::sort(actual.begin(), actual.end(), [](const QPair<QString, int> &l,
                                               const QPair<QString, int> &r) {
        return l.second > r.second;
    });


    QStringList actualOut;
    for (const auto& s : actual) {
        actualOut << s.first;
    }
    return actualOut;
}

void KFuzzyMatcherTest::testMatch()
{
    QFETCH(QString, pattern);
    QFETCH(QStringList, input);
    QFETCH(QStringList, expected);
    QFETCH(int, size);

    const auto actual = matchHelper<KFuzzyMatcher::match>(pattern, input);

    QCOMPARE(actual.size(), size);
    QCOMPARE(actual, expected);
}

void KFuzzyMatcherTest::testMatchSequential_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<int>("size");

    QTest::newRow("sort") << QStringLiteral("sort")
                          << QStringList{
                                QStringLiteral("Sort"),
                                QStringLiteral("Some other right test"),
                                QStringLiteral("Soup rate"),
                                QStringLiteral("Someother"),
                                QStringLiteral("irrelevant"),
                              }
                          << QStringList{
                                QStringLiteral("Sort"),
                                QStringLiteral("Some other right test"),
                                QStringLiteral("Soup rate"),
                              }
                          << 3;
}

void KFuzzyMatcherTest::testMatchSequential()
{
    QFETCH(QString, pattern);
    QFETCH(QStringList, input);
    QFETCH(QStringList, expected);
    QFETCH(int, size);

    const auto actual = matchHelper<KFuzzyMatcher::matchSequential>(pattern, input);

    QCOMPARE(actual.size(), size);
    QCOMPARE(actual, expected);
}

void KFuzzyMatcherTest::testToFuzzyMatchedDisplayString_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<QString>("tag");
    QTest::addColumn<QString>("tagClose");

    QTest::newRow("HelloBold") << QStringLiteral("Hlo")
                               << QStringLiteral("Hello")
                               << QStringLiteral("<b>H</b>e<b>l</b>l<b>o</b>")
                               << QStringLiteral("<b>")
                               << QStringLiteral("</b>");

    QTest::newRow("HelloItalic") << QStringLiteral("Hlo")
                                 << QStringLiteral("Hello")
                                 << QStringLiteral("<i>H</i>e<i>l</i>l<i>o</i>")
                                 << QStringLiteral("<i>")
                                 << QStringLiteral("</i>");
}

void KFuzzyMatcherTest::testToFuzzyMatchedDisplayString()
{
    QFETCH(QString, pattern);
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QFETCH(QString, tag);
    QFETCH(QString, tagClose);

    QString actual = KFuzzyMatcher::toFuzzyMatchedDisplayString(pattern, input, tag, tagClose);

    QCOMPARE(actual, expected);
    QCOMPARE(input, expected);
}
