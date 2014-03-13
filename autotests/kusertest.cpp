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
    // make sure KUser::currentUserId() and KUser::curretEffectiveUserId() work
    KUserId currentUser = KUserId::currentUserId();
    QVERIFY(currentUser.isValid());
    KUserId currentEffectiveUser = KUserId::currentEffectiveUserId();
    QVERIFY(currentEffectiveUser.isValid());
    // these should be the same since this is not a setuid program
    QVERIFY(currentUser == currentEffectiveUser);

    KUser kuser(currentUser);
    //now get the same user from his name
    QString userName = kuser.loginName();
    qDebug("Current user: %s, id: %s", qPrintable(userName), qPrintable(currentUser.toString()));
    QVERIFY(!userName.isEmpty());
    KUserId currentUserFromStr = KUserId::fromName(userName);
    QVERIFY(currentUserFromStr.isValid());
    KUserId currentUserCopyFromKUser = kuser.userId();
    QVERIFY(currentUserCopyFromKUser.isValid());
    KUserId invalid;
    QVERIFY(!invalid.isValid());
#ifdef Q_OS_WIN
    KUserId invalid2(nullptr);
#else
    KUserId invalid2(-1);
#endif
    QVERIFY(!invalid2.isValid());
    // I guess it is safe to assume no user with this name exists
    KUserId invalid3 = KUserId::fromName("This_user_does_not_exist");
    QVERIFY(!invalid3.isValid());

    //check comparison
    QVERIFY(invalid == KUserId());
    QVERIFY(invalid == invalid2);
    QVERIFY(invalid == invalid3);
    QVERIFY(currentUser == currentUserFromStr);
    QVERIFY(currentUser == currentEffectiveUser);
    QVERIFY(currentUser == currentUserCopyFromKUser);
    QVERIFY(currentUser != invalid);
    QVERIFY(currentUser != invalid2);
    QVERIFY(currentUser != invalid3);
    QVERIFY(invalid != currentUser);
    //Copy constructor and assignment
    KUserId currentUserCopy = currentUser;
    QVERIFY(currentUser == currentUserCopy);
    QVERIFY(currentUser == KUserId(currentUser));
    QVERIFY(currentEffectiveUser == KUserId(currentUser));
}

void KUserTest::testKGroupId()
{
    // make sure KGroup::currentGroupId() and KGroup::curretEffectiveGroupId() work
    KGroupId currentGroup = KGroupId::currentGroupId();
    QVERIFY(currentGroup.isValid());
    KGroupId currentEffectiveGroup = KGroupId::currentEffectiveGroupId();
    QVERIFY(currentEffectiveGroup.isValid());
    // these should be the same since this is not a setuid program
    QVERIFY(currentGroup == currentEffectiveGroup);

    //now get the same Group from his name
    QString groupName = KUserGroup(currentGroup).name();
    qDebug("Current group: %s, id: %s", qPrintable(groupName), qPrintable(currentGroup.toString()));
    QVERIFY(!groupName.isEmpty());
    KGroupId currentGroupFromStr = KGroupId::fromName(groupName);
    QVERIFY(currentGroupFromStr.isValid());
    KGroupId invalid;
    QVERIFY(!invalid.isValid());
#ifdef Q_OS_WIN
    KGroupId invalid2(nullptr);
#else
    KGroupId invalid2(-1);
#endif
    QVERIFY(!invalid2.isValid());
    // I guess it is safe to assume no Group with this name exists
    KGroupId invalid3 = KGroupId::fromName("This_Group_does_not_exist");
    QVERIFY(!invalid3.isValid());

    //check comparison
    QVERIFY(invalid == KGroupId());
    QVERIFY(invalid == invalid2);
    QVERIFY(invalid == invalid3);
    QVERIFY(currentGroup == currentGroupFromStr);
    QVERIFY(invalid != currentGroup);
    QVERIFY(currentGroup != invalid);
    QVERIFY(currentGroup != invalid2);
    QVERIFY(currentGroup != invalid3);
    //Copy constructor and assignment
    KGroupId currentGroupCopy = currentGroup;
    QVERIFY(currentGroup == currentGroupCopy);
    QVERIFY(currentGroup == KGroupId(currentGroup));
    QVERIFY(currentEffectiveGroup == KGroupId(currentGroup));
}


QTEST_MAIN(KUserTest)

#include "kusertest.moc"
