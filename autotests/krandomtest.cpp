/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2016 Michael Pyne <mpyne@kde.org>
    SPDX-FileCopyrightText: 2016 Arne Spiegelhauer <gm2.asp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <krandom.h>
#include <krandomsequence.h>
#include <stdlib.h>

#include <QTest>
#include <QThread>

#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>
#include <QVarLengthArray>

#include <algorithm>
#include <iostream>

typedef QVarLengthArray<int> intSequenceType;

static const char *binpath;

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 75)
static bool seqsAreEqual(const intSequenceType &l, const intSequenceType &r)
{
    if (l.size() != r.size()) {
        return false;
    }
    const intSequenceType::const_iterator last(l.end());

    intSequenceType::const_iterator l_first(l.begin());
    intSequenceType::const_iterator r_first(r.begin());

    while (l_first != last && *l_first == *r_first) {
        l_first++;
        r_first++;
    }

    return l_first == last;
}
#endif

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 72)
// Fills seq with random bytes produced by a new process. Seq should already
// be sized to the needed amount of random numbers.
static bool getChildRandSeq(intSequenceType &seq)
{
    QProcess subtestProcess;

    // Launch a separate process to generate random numbers to test first-time
    // seeding.
    subtestProcess.start(QLatin1String(binpath), QStringList() << QString::number(seq.count()));
    subtestProcess.waitForFinished();

    QTextStream childStream(subtestProcess.readAllStandardOutput());

    std::generate(seq.begin(), seq.end(), [&]() {
        int temp;
        childStream >> temp;
        return temp;
    });

    char c;
    childStream >> c;
    return c == '@' && childStream.status() == QTextStream::Ok;
}
#endif

class KRandomTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 72)
    void test_random();
#endif
    void test_randomString();
    void test_randomStringThreaded();
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 75)
    void test_KRS();
#endif
    void test_shuffle();
};

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 72)
void KRandomTest::test_random()
{
    int testValue = KRandom::random();

    QVERIFY(testValue >= 0);
    QVERIFY(testValue < RAND_MAX);

    // Verify seeding results in different numbers across different procs
    // See bug 362161
    intSequenceType out1(10);
    intSequenceType out2(10);

    QVERIFY(getChildRandSeq(out1));
    QVERIFY(getChildRandSeq(out2));

    QVERIFY(!seqsAreEqual(out1, out2));
}
#endif

void KRandomTest::test_randomString()
{
    const int desiredLength = 12;
    const QString testString = KRandom::randomString(desiredLength);
    const QRegularExpression outputFormat(QRegularExpression::anchoredPattern(QStringLiteral("[A-Za-z0-9]+")));
    const QRegularExpressionMatch match = outputFormat.match(testString);

    QCOMPARE(testString.length(), desiredLength);
    QVERIFY(match.hasMatch());
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 75)
void KRandomTest::test_KRS()
{
    using std::all_of;
    using std::generate;

    const int maxInt = 50000;
    KRandomSequence krs1;
    KRandomSequence krs2;
    intSequenceType out1(10);
    intSequenceType out2(10);

    generate(out1.begin(), out1.end(), [&]() {
        return krs1.getInt(maxInt);
    });
    generate(out2.begin(), out2.end(), [&]() {
        return krs2.getInt(maxInt);
    });
    QVERIFY(!seqsAreEqual(out1, out2));
    QVERIFY(all_of(out1.begin(), out1.end(), [&](int x) {
        return x < maxInt;
    }));
    QVERIFY(all_of(out2.begin(), out2.end(), [&](int x) {
        return x < maxInt;
    }));

    // Compare same-seed
    krs1.setSeed(5000);
    krs2.setSeed(5000);

    generate(out1.begin(), out1.end(), [&]() {
        return krs1.getInt(maxInt);
    });
    generate(out2.begin(), out2.end(), [&]() {
        return krs2.getInt(maxInt);
    });
    QVERIFY(seqsAreEqual(out1, out2));
    QVERIFY(all_of(out1.begin(), out1.end(), [&](int x) {
        return x < maxInt;
    }));
    QVERIFY(all_of(out2.begin(), out2.end(), [&](int x) {
        return x < maxInt;
    }));

    // Compare same-seed and assignment ctor

    krs1 = KRandomSequence(8000);
    krs2 = KRandomSequence(8000);

    generate(out1.begin(), out1.end(), [&]() {
        return krs1.getInt(maxInt);
    });
    generate(out2.begin(), out2.end(), [&]() {
        return krs2.getInt(maxInt);
    });
    QVERIFY(seqsAreEqual(out1, out2));
    QVERIFY(all_of(out1.begin(), out1.end(), [&](int x) {
        return x < maxInt;
    }));
    QVERIFY(all_of(out2.begin(), out2.end(), [&](int x) {
        return x < maxInt;
    }));
}
#endif

void KRandomTest::test_shuffle()
{
    {
        QRandomGenerator rg(1);
        QList<int> list = {1, 2, 3, 4, 5};
        const QList<int> shuffled = {5, 2, 4, 3, 1};
        KRandom::shuffle(list, &rg);
        QCOMPARE(list, shuffled);
    }

    {
        QRandomGenerator rg(1);
        QVector<int> vector = {1, 2, 3, 4, 5};
        const QVector<int> shuffled = {5, 2, 4, 3, 1};
        KRandom::shuffle(vector, &rg);
        QCOMPARE(vector, shuffled);
    }

    {
        QRandomGenerator rg(1);
        std::vector<int> std_vector = {1, 2, 3, 4, 5};
        const std::vector<int> shuffled = {5, 2, 4, 3, 1};
        KRandom::shuffle(std_vector, &rg);
        QCOMPARE(std_vector, shuffled);
    }
}

class KRandomTestThread : public QThread
{
protected:
    void run() override
    {
        result = KRandom::randomString(32);
    };

public:
    QString result;
};

void KRandomTest::test_randomStringThreaded()
{
    static const int size = 5;
    KRandomTestThread *threads[size];
    for (int i = 0; i < size; ++i) {
        threads[i] = new KRandomTestThread();
        threads[i]->start();
    }
    QSet<QString> results;
    for (int i = 0; i < size; ++i) {
        threads[i]->wait(2000);
        results.insert(threads[i]->result);
    }
    // each thread should have returned a unique result
    QCOMPARE(results.size(), size);
    for (int i = 0; i < size; ++i) {
        delete threads[i];
    }
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 72)
// Used by getChildRandSeq... outputs random numbers to stdout and then
// exits the process.
static void childGenRandom(int count)
{
    // No logic to 300, just wanted to avoid it accidentally being 2.4B...
    if (count <= 0 || count > 300) {
        exit(-1);
    }

    while (--count > 0) {
        std::cout << KRandom::random() << ' ';
    }

    std::cout << KRandom::random() << '@';
    exit(0);
}
#endif

// Manually implemented to dispatch to child process if needed to support
// subtests
int main([[maybe_unused]] int argc, char *argv[])
{
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 72)
    if (argc > 1) {
        childGenRandom(std::atoi(argv[1]));
        Q_UNREACHABLE();
    }
#endif

    binpath = argv[0];
    KRandomTest randomTest;
    return QTest::qExec(&randomTest);
}

#include "krandomtest.moc"
