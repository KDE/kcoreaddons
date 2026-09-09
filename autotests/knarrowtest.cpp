// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#include <QTest>

#include "knarrow.h"

class KNarrowTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test()
    {
        // Only make sure it works. The qCriticals make this annoying to test.
        QCOMPARE(KNarrow::narrow<int>(30), 30L);
    }
};

QTEST_MAIN(KNarrowTest)

#include "knarrowtest.moc"
