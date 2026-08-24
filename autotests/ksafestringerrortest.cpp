// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

#include <QTest>

#include "ksafestringerror.h"

class KSafeStringErrorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testStrerror()
    {
        // Since we don't know what the underlying impl will return we mostly just rely on sanitizers to catch issues here.
        QVERIFY(strlen(KSafeStringError::strerror(EINVAL)) > 0);
        qDebug() << KSafeStringError::strerror(0);
        qDebug() << KSafeStringError::strerror(EINVAL);
        qDebug() << KSafeStringError::strerror(-EINVAL);
        qDebug() << KSafeStringError::strerror(std::numeric_limits<int>::max());
    }

    void testQString()
    {
        QVERIFY(KSafeStringError::strerrorQString(EINVAL).length() > 0);
        qDebug() << KSafeStringError::strerrorQString(0);
        qDebug() << KSafeStringError::strerrorQString(EINVAL);
        qDebug() << KSafeStringError::strerrorQString(-EINVAL);
        qDebug() << KSafeStringError::strerrorQString(std::numeric_limits<int>::max());
    }
};

QTEST_MAIN(KSafeStringErrorTest)

#include "ksafestringerrortest.moc"
