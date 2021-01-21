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

void KFuzzyMatcherTest::testMatch()
{
    // string , score
    QVector<QString> l = {QStringLiteral("Sort"),
                          QStringLiteral("Some other right test"),
                          QStringLiteral("Soup rate"),
                          QStringLiteral("Someother"),
                          QStringLiteral("irrelevant")};

    const QString pattern = QStringLiteral("sort");

    // <string, score>
    QVector<QPair<QString, int>> actual;

    for (int i = 0; i < l.size(); ++i) {
        int score = 0;
        bool res = KFuzzyMatcher::match(pattern, l.at(i), score);
        if (res) {
            actual.push_back({l.at(i), score});
        }
    }

    // two strings filtered out
    QCOMPARE(actual.size(), 3);

    // sort descending based on score
    std::sort(actual.begin(), actual.end(), [](const QPair<QString, int> &l, const QPair<QString, int> &r) {
        return l.second > r.second;
    });

    const QVector<QString> expected = {
        QStringLiteral("Some other right test"), QStringLiteral("Sort"), QStringLiteral("Soup rate")};

    for (int i = 0; i < actual.size(); ++i) {
        QCOMPARE(actual.at(i).first, expected.at(i));
    }
}

void KFuzzyMatcherTest::testMatch2()
{
    const QString pattern = QStringLiteral("kaapp");
    const QVector<QString> strs = {QStringLiteral("kateapp.cpp"),
                                   QStringLiteral("kate_application"),
                                   QStringLiteral("kateapp.h"),
                                   QStringLiteral("katepap.c")};

    QVector<QString> matched;
    for (const auto &str : strs) {
        int score = 0;
        auto res = KFuzzyMatcher::match(pattern, str, score);
        if (res) {
            matched.push_back(str);
        }
    }

    QCOMPARE(matched.size(), 3);
    QCOMPARE(matched.contains(QStringLiteral("kateapp.cpp")), true);
    QCOMPARE(matched.contains(QStringLiteral("katepap.c")), false);
}

void KFuzzyMatcherTest::testMatchSequential()
{
    // string , score
    QVector<QString> l = {QStringLiteral("Sort"),
                          QStringLiteral("Some other right test"),
                          QStringLiteral("Soup rate"),
                          QStringLiteral("Someother"),
                          QStringLiteral("irrelevant")};

    const QString pattern = QStringLiteral("sort");

    // <string, score>
    QVector<QPair<QString, int>> actual;

    for (int i = 0; i < l.size(); ++i) {
        int score = 0;
        bool res = KFuzzyMatcher::matchSequential(pattern, l.at(i), score);
        if (res) {
            actual.push_back({l.at(i), score});
        }
    }

    // two strings filtered out
    QCOMPARE(actual.size(), 3);

    // sort descending based on score
    std::sort(actual.begin(), actual.end(), [](const QPair<QString, int> &l, const QPair<QString, int> &r) {
        return l.second > r.second;
    });

    const QVector<QString> expected = {
        QStringLiteral("Sort"), QStringLiteral("Some other right test"), QStringLiteral("Soup rate")};

    for (int i = 0; i < actual.size(); ++i) {
        QCOMPARE(actual.at(i).first, expected.at(i));
    }
}

void KFuzzyMatcherTest::testToFuzzyMatchedDisplayString()
{
    QString pattern = QStringLiteral("Hlo");
    QString str = QStringLiteral("Hello");
    KFuzzyMatcher::toFuzzyMatchedDisplayString(pattern, str, QStringLiteral("<b>"), QStringLiteral("</b>"));
    QCOMPARE(str, QStringLiteral("<b>H</b>e<b>l</b>l<b>o</b>"));
}
