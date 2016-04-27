/* This file is part of the KDE libraries
    Copyright (c) 2016 Michael Pyne <mpyne@kde.org>

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

#include <krandom.h>
#include <krandomsequence.h>
#include <stdlib.h>

#include <QtTest>

#include <QObject>
#include <QString>
#include <QRegExp>
#include <QVarLengthArray>

#include <algorithm>

typedef QVarLengthArray<int> intSequenceType;

static bool seqsAreEqual(const intSequenceType &l, const intSequenceType &r)
{
    if(l.size() != r.size()) {
        return false;
    }
    const intSequenceType::const_iterator last(l.end());

    intSequenceType::const_iterator l_first(l.begin());
    intSequenceType::const_iterator r_first(r.begin());

    while(l_first != last && *l_first == *r_first) {
        l_first++; r_first++;
    }

    return l_first == last;
}

class KRandomTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_random();
    void test_randomString();
    void test_KRS();
};

void KRandomTest::test_random()
{
    int testValue = KRandom::random();

    QVERIFY(testValue >= 0);
    QVERIFY(testValue < RAND_MAX);
}

void KRandomTest::test_randomString()
{
    const int desiredLength = 12;
    const QRegExp outputFormat("[A-Za-z0-9]+");
    QString testString = KRandom::randomString(desiredLength);

    QCOMPARE(testString.length(), desiredLength);
    QVERIFY(outputFormat.exactMatch(testString));
}

void KRandomTest::test_KRS()
{
    using std::generate;
    using std::all_of;

    const int maxInt = 50000;
    KRandomSequence krs1, krs2;
    intSequenceType out1(10), out2(10);

    generate(out1.begin(), out1.end(), [&]() { return krs1.getInt(maxInt); });
    generate(out2.begin(), out2.end(), [&]() { return krs2.getInt(maxInt); });
    QVERIFY(!seqsAreEqual(out1, out2));
    QVERIFY(all_of(out1.begin(), out1.end(), [&](int x) { return x < maxInt; }));
    QVERIFY(all_of(out2.begin(), out2.end(), [&](int x) { return x < maxInt; }));

    // Compare same-seed
    krs1.setSeed(5000);
    krs2.setSeed(5000);

    generate(out1.begin(), out1.end(), [&]() { return krs1.getInt(maxInt); });
    generate(out2.begin(), out2.end(), [&]() { return krs2.getInt(maxInt); });
    QVERIFY(seqsAreEqual(out1, out2));
    QVERIFY(all_of(out1.begin(), out1.end(), [&](int x) { return x < maxInt; }));
    QVERIFY(all_of(out2.begin(), out2.end(), [&](int x) { return x < maxInt; }));

    // Compare same-seed and assignment ctor

    krs1 = KRandomSequence(8000);
    krs2 = KRandomSequence(8000);

    generate(out1.begin(), out1.end(), [&]() { return krs1.getInt(maxInt); });
    generate(out2.begin(), out2.end(), [&]() { return krs2.getInt(maxInt); });
    QVERIFY(seqsAreEqual(out1, out2));
    QVERIFY(all_of(out1.begin(), out1.end(), [&](int x) { return x < maxInt; }));
    QVERIFY(all_of(out2.begin(), out2.end(), [&](int x) { return x < maxInt; }));
}

QTEST_MAIN(KRandomTest)

#include "krandomtest.moc"
