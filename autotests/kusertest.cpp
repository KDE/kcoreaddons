/*
*  Copyright (C) 2014 Alex Richardson <arichardson.kde@gmail.com>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
*/
#include <QTest>

#include "kuser.h"

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <sddl.h>
#endif

class KUserTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void testKUserId();
    void testKGroupId();
};


void KUserTest::testKUserId()
{
    KUser currentUser;
    KUserId currentUserId(currentUser.uid());
    KUserId currentUserIdCopy = currentUserId;
    KUserId currentUserIdFromStr = KUserId::fromName(currentUser.loginName());
    KUserId invalid;
#ifdef Q_OS_WIN
    KUserId invalid2(nullptr);
#else
    KUserId invalid2(-1);
#endif
    // I guess it is safe to assume no user with this name exists
    KUserId invalid3 = KUserId::fromName("This_user_does_not_exist");
    
    QVERIFY(!invalid.isValid());
    QVERIFY(!invalid2.isValid());
    QVERIFY(!invalid3.isValid());
    QVERIFY(invalid == KUserId());
    QVERIFY(invalid == invalid2);
    QVERIFY(invalid == invalid3);
    QVERIFY(currentUserId.isValid());
    QVERIFY(currentUserIdCopy.isValid());
    QVERIFY(currentUserIdFromStr.isValid());
    QVERIFY(currentUserId == currentUserIdCopy);
    QVERIFY(currentUserId == currentUserIdFromStr);
    QVERIFY(currentUserId == KUserId(currentUserId));
    QVERIFY(currentUserId != invalid);
    QVERIFY(currentUserId != invalid2);
    QVERIFY(currentUserId != invalid3);
}

void KUserTest::testKGroupId()
{
    KGroupId invalid;
#ifdef Q_OS_WIN
    KGroupId invalid2(nullptr);
#else
    KGroupId invalid2(-1);
#endif
    QVERIFY(!invalid.isValid());
    QVERIFY(!invalid2.isValid());
    QVERIFY(invalid == invalid2);
}


QTEST_MAIN(KUserTest)

#include "kusertest.moc"
