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
#include <QDebug>

#include "kuser.h"

namespace QTest {
    template<>
    char* toString(const KUserId &id)
    {
        return qstrdup(id.toString().toLocal8Bit().data());
    }
    template<>
    char* toString(const KGroupId &id)
    {
        return qstrdup(id.toString().toLocal8Bit().data());
    }
}

class KUserTest : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void testKUser();
    void testKUserGroup();
    void testKUserId();
    void testKGroupId();
};

static inline void printUserInfo(KUser user)
{
    qDebug() << "Login name:" << user.loginName();
    qDebug() << "Full name:" << user.fullName();
    qDebug() << "User ID:" << user.userId().toString();
    qDebug() << "Group ID:" << user.groupId().toString();
    qDebug() << "Home dir:" << user.homeDir();
    qDebug() << "Superuser:" << user.isSuperUser();
    qDebug() << "Shell: " << user.shell();
    qDebug() << "Face icon path:" << user.faceIconPath();
    qDebug() << "Groups:" << user.groupNames();
    qDebug();
}

void KUserTest::testKUser()
{
    KUser user(KUser::UseRealUserID);
    KUser effectiveUser(KUser::UseRealUserID);
    QVERIFY(user.isValid());
    QVERIFY(effectiveUser.isValid());
    QCOMPARE(user, effectiveUser); // should be the same, no suid
    QVERIFY(user.groupId().isValid());
    QCOMPARE(user.groupId(), KGroupId::currentGroupId());
    QVERIFY(!user.groups().isEmpty()); // user must be in at least one group
    QVERIFY(!user.groupNames().isEmpty()); // user must be in at least one group
    QCOMPARE(user.groups().size(), user.groupNames().size());

    QStringList allUserNames = KUser::allUserNames();
    QList<KUser> allUsers = KUser::allUsers();
    QVERIFY(!allUserNames.isEmpty());
    QVERIFY(!allUsers.isEmpty());
    QCOMPARE(allUsers.size(), allUserNames.size());
    // check that the limiting works
    QCOMPARE(user.groups(1).size(), 1);
    QCOMPARE(user.groupNames(1).size(), 1);
    qDebug() << "All users: " << allUserNames;
    // check that the limiting works
    QCOMPARE(KUser::allUserNames(1).size(), 1);
    QCOMPARE(KUser::allUsers(1).size(), 1);
    // We can't test the KUser properties, since they differ on each system
    // instead just print them all out, this can be verified by the person running the test
    printUserInfo(user);
#if 0 //enable this if you think that KUser might not be working correctly
    Q_FOREACH(const KUser& u, allUsers) {
        printUserInfo(u);
    }
#endif

    // test operator==
    KUser invalidKUser = KUser(KUserId());
    QVERIFY(invalidKUser != invalidKUser); //invalid never equal
    QVERIFY(invalidKUser != user);
    QVERIFY(user != invalidKUser);    // now test the other way around
    QCOMPARE(user, user);

    // make sure we don't crash when accessing properties of an invalid instace
    QCOMPARE(invalidKUser.faceIconPath(), QString());
    QCOMPARE(invalidKUser.fullName(), QString());
    QCOMPARE(invalidKUser.groupId(), KGroupId());
    QCOMPARE(invalidKUser.groupNames(), QStringList());
    QCOMPARE(invalidKUser.groups(), QList<KUserGroup>());
    QCOMPARE(invalidKUser.homeDir(), QString());
    QCOMPARE(invalidKUser.isSuperUser(), false);
    QCOMPARE(invalidKUser.loginName(), QString());
    QCOMPARE(invalidKUser.shell(), QString());
    QCOMPARE(invalidKUser.userId(), KUserId());
    QCOMPARE(invalidKUser.userId(), KUserId());
    QCOMPARE(invalidKUser.property(KUser::RoomNumber), QVariant());
}

void KUserTest::testKUserGroup()
{
    KUserGroup group(KUser::UseRealUserID);
    KUserGroup effectiveUser(KUser::UseEffectiveUID);
    QVERIFY(group.isValid());
    QVERIFY(effectiveUser.isValid());
    QCOMPARE(group, effectiveUser); // should be the same, no suid
#ifdef Q_OS_WIN
    // on Windows the special group "None" has no members (often the only group that exists)
    if (group.name() != QLatin1String("None")) {
#else
    {
#endif
        QStringList groupUserNames = group.userNames();
        QList<KUser> groupUsers = group.users();
        QVERIFY(!groupUsers.isEmpty()); // group must have at least one user (the current user)
        QVERIFY(!groupUserNames.isEmpty()); // group must have at least one user (the current user)
        QCOMPARE(groupUsers.size(), groupUserNames.size());
        // check that the limiting works
        QCOMPARE(group.users(1).size(), 1);
        QCOMPARE(group.userNames(1).size(), 1);
    }

    QStringList allGroupNames = KUserGroup::allGroupNames();
    QList<KUserGroup> allGroups = KUserGroup::allGroups();
    QVERIFY(!allGroupNames.isEmpty());
    QVERIFY(!allGroups.isEmpty());
    QCOMPARE(allGroups.size(), allGroupNames.size());
    qDebug() << "All groups: " << allGroupNames;
    // check that the limiting works
    QCOMPARE(KUserGroup::allGroupNames(1).size(), 1);
    QCOMPARE(KUserGroup::allGroups(1).size(), 1);
    // We can't test the KUser properties, since they differ on each system
    // instead just print them all out, this can be verified by the person running the test
    qDebug().nospace() << "Current group: " << group.name() << ", group ID =" << group.groupId().toString();
#if 0 //enable this if you think that KUser might not be working correctly
    for (int i = 0; i < allGroups.size(); ++i) {
        qDebug().nospace() << "Group " << i << ": name = " << allGroups[i].name()
            << ", group ID =" << allGroups[i].groupId().toString();
        qDebug() << allGroups[i].name() << "members:" << allGroups[i].userNames();
    }
#endif
    // test operator==
    KUserGroup invalidKUserGroup = KUserGroup(KGroupId());
    QVERIFY(invalidKUserGroup != invalidKUserGroup); //invalid never equal
    QVERIFY(invalidKUserGroup != group);
    QVERIFY(group != invalidKUserGroup);    // now test the other way around
    QCOMPARE(group, group);

    // make sure we don't crash when accessing an invalid KUserGroup
    QCOMPARE(invalidKUserGroup.groupId(), KGroupId());
    QCOMPARE(invalidKUserGroup.name(), QString());
    QCOMPARE(invalidKUserGroup.userNames(), QStringList());
    QCOMPARE(invalidKUserGroup.users(), QList<KUser>());

}

void KUserTest::testKUserId()
{
    // make sure KUser::currentUserId() and KUser::curretEffectiveUserId() work
    KUserId currentUser = KUserId::currentUserId();
    QVERIFY(currentUser.isValid());
    KUserId currentEffectiveUser = KUserId::currentEffectiveUserId();
    QVERIFY(currentEffectiveUser.isValid());
    // these should be the same since this is not a setuid program
    QCOMPARE(currentUser, currentEffectiveUser);

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
    QCOMPARE(invalid, KUserId());
    QCOMPARE(invalid, invalid2);
    QCOMPARE(invalid, invalid3);
    QCOMPARE(currentUser, currentUserFromStr);
    QCOMPARE(currentUser, currentEffectiveUser);
    QCOMPARE(currentUser, currentUserCopyFromKUser);
    QVERIFY(currentUser != invalid);
    QVERIFY(currentUser != invalid2);
    QVERIFY(currentUser != invalid3);
    QVERIFY(invalid != currentUser);
    //Copy constructor and assignment
    KUserId currentUserCopy = currentUser;
    QCOMPARE(currentUser, currentUserCopy);
    QCOMPARE(currentUser, KUserId(currentUser));
    QCOMPARE(currentEffectiveUser, KUserId(currentUser));
}

void KUserTest::testKGroupId()
{
    // make sure KGroup::currentGroupId() and KGroup::curretEffectiveGroupId() work
    KGroupId currentGroup = KGroupId::currentGroupId();
    QVERIFY(currentGroup.isValid());
    KGroupId currentEffectiveGroup = KGroupId::currentEffectiveGroupId();
    QVERIFY(currentEffectiveGroup.isValid());
    // these should be the same since this is not a setuid program
    QCOMPARE(currentGroup, currentEffectiveGroup);

    //now get the same Group from his name
    KUserGroup kuserGroup(currentGroup);
    QString groupName = kuserGroup.name();
    qDebug("Current group: %s, id: %s", qPrintable(groupName), qPrintable(currentGroup.toString()));
    QVERIFY(!groupName.isEmpty());
    KGroupId currentGroupFromStr = KGroupId::fromName(groupName);
    QVERIFY(currentGroupFromStr.isValid());
    KGroupId currentGroupCopyFromKUserGroup = kuserGroup.groupId();
    QVERIFY(currentGroupCopyFromKUserGroup.isValid());
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
    QCOMPARE(invalid, KGroupId());
    QCOMPARE(invalid, invalid2);
    QCOMPARE(invalid, invalid3);
    QCOMPARE(currentGroup, currentGroupFromStr);
    QCOMPARE(currentGroup, currentEffectiveGroup);
    QCOMPARE(currentGroup, currentGroupCopyFromKUserGroup);
    QVERIFY(invalid != currentGroup);
    QVERIFY(currentGroup != invalid);
    QVERIFY(currentGroup != invalid2);
    QVERIFY(currentGroup != invalid3);
    //Copy constructor and assignment
    KGroupId currentGroupCopy = currentGroup;
    QCOMPARE(currentGroup, currentGroupCopy);
    QCOMPARE(currentGroup, KGroupId(currentGroup));
    QCOMPARE(currentEffectiveGroup, KGroupId(currentGroup));
}


QTEST_MAIN(KUserTest)

#include "kusertest.moc"
