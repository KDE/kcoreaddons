// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#include <QTest>

#include "ksafestrerror.h"

class KSafeStrerrorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testStrerror()
    {
        // Since we don't know what the underlying impl will return we mostly just rely on sanitizers to catch issues here.
        QVERIFY(strlen(KSafeStrerror::strerror(EINVAL)) > 0);
        qDebug() << KSafeStrerror::strerror(0);
        qDebug() << KSafeStrerror::strerror(EINVAL);
        qDebug() << KSafeStrerror::strerror(-EINVAL);
        qDebug() << KSafeStrerror::strerror(std::numeric_limits<int>::max());
    }

    void testQString()
    {
        QVERIFY(KSafeStrerror::strerrorQString(EINVAL).length() > 0);
        qDebug() << KSafeStrerror::strerrorQString(0);
        qDebug() << KSafeStrerror::strerrorQString(EINVAL);
        qDebug() << KSafeStrerror::strerrorQString(-EINVAL);
        qDebug() << KSafeStrerror::strerrorQString(std::numeric_limits<int>::max());
    }
};

QTEST_MAIN(KSafeStrerrorTest)

#include "ksafestrerrortest.moc"
