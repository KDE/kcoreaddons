// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#include <QTest>

#include "knarrow.h"

class KNarrowExceptionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test()
    {
#if !defined(__cpp_exceptions)
        static_assert(false, "Must have exceptions enabled to test exceptions");
#endif

        QCOMPARE(KNarrow::narrow<int>(0), 0);
        QCOMPARE(KNarrow::narrow<unsigned int>(30), 30U);
        QCOMPARE(KNarrow::narrow<int>(-30LL), -30);

        // Simply does not fit in output type
        QVERIFY_THROWS_EXCEPTION(KNarrow::NarrowingError, (void)KNarrow::narrow<int>(std::numeric_limits<long long>::max()));
        // Changed signedness and value is out of range for output type
        QVERIFY_THROWS_EXCEPTION(KNarrow::NarrowingError, (void)KNarrow::narrow<int>(std::numeric_limits<unsigned int>::max()));
    }

    void testExpected()
    {
#if !defined(__cpp_lib_expected)
        static_assert(false, "Must have std::expected enabled to test expected");
#endif

        QCOMPARE(KNarrow::expectingNarrow<int>(0), 0);
        QCOMPARE(KNarrow::expectingNarrow<unsigned int>(30), 30U);
        QCOMPARE(KNarrow::expectingNarrow<int>(-30LL), -30);

        // Simply does not fit in output type
        QCOMPARE(KNarrow::expectingNarrow<int>(std::numeric_limits<long long>::max()).error(), KNarrow::NarrowingErrorType::ValueChanged);
        // Changed signedness and value is out of range for output type
        QCOMPARE(KNarrow::expectingNarrow<int>(std::numeric_limits<unsigned int>::max()).error(), KNarrow::NarrowingErrorType::ValueMeaningChanged);
    }
};

QTEST_MAIN(KNarrowExceptionTest)

#include "knarrowexceptiontest.moc"
