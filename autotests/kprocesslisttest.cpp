/*
 *  This file is part of the KDE project
 *  Copyright (C) 2019 David Hallas <david@davidhallas.dk>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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

#include "kprocesslisttest.h"
#include "kprocesslist.h"
#include "kuser.h"
#include <QCoreApplication>
#include <QTest>
#include <algorithm>

namespace {

QString getTestExeName()
{
    static QString testExeName = QCoreApplication::instance()->applicationFilePath().section(QLatin1Char('/'), -1);
    return testExeName;
}

}

QTEST_MAIN(KProcessListTest)

void KProcessListTest::testKProcessInfoConstructionAssignment()
{
    KProcessList::KProcessInfo processInfoDefaultConstructed;
    QVERIFY(processInfoDefaultConstructed.isValid() == false);
    const qint64 pid(42);
    const QString name(QStringLiteral("/bin/some_exe"));
    const QString user(QStringLiteral("some_user"));
    KProcessList::KProcessInfo processInfo(pid, name, user);
    QVERIFY(processInfo.isValid() == true);
    QCOMPARE(processInfo.pid(), pid);
    QCOMPARE(processInfo.name(), name);
    QCOMPARE(processInfo.user(), user);
    KProcessList::KProcessInfo processInfoCopy(processInfo);
    QVERIFY(processInfoCopy.isValid() == true);
    QCOMPARE(processInfoCopy.pid(), pid);
    QCOMPARE(processInfoCopy.name(), name);
    QCOMPARE(processInfoCopy.user(), user);
    KProcessList::KProcessInfo processInfoAssignment;
    processInfoAssignment = processInfo;
    QVERIFY(processInfoAssignment.isValid() == true);
    QCOMPARE(processInfoAssignment.pid(), pid);
    QCOMPARE(processInfoAssignment.name(), name);
    QCOMPARE(processInfoAssignment.user(), user);
}

void KProcessListTest::testProcessInfoList()
{
    KProcessList::KProcessInfoList processInfoList = KProcessList::processInfoList();
    QVERIFY(processInfoList.empty() == false);
    auto testProcessIterator = std::find_if(processInfoList.begin(), processInfoList.end(), [](const KProcessList::KProcessInfo& info)
    {
        return info.command().endsWith(QLatin1String("/") + getTestExeName());
    });
    QVERIFY(testProcessIterator != processInfoList.end());
    const auto& processInfo = *testProcessIterator;
    QVERIFY(processInfo.isValid() == true);
    QVERIFY(processInfo.command().endsWith(QLatin1String("/") + getTestExeName()));
    QCOMPARE(processInfo.name(), getTestExeName());
    QCOMPARE(processInfo.pid(), QCoreApplication::applicationPid());
    QCOMPARE(processInfo.user(), KUser().loginName());
}

void KProcessListTest::testProcessInfo()
{
    const qint64 testExePid = QCoreApplication::applicationPid();
    KProcessList::KProcessInfo processInfo = KProcessList::processInfo(testExePid);
    QVERIFY(processInfo.isValid() == true);
    QVERIFY(processInfo.command().endsWith(QLatin1String("/") + getTestExeName()));
    QCOMPARE(processInfo.pid(), testExePid);
    QCOMPARE(processInfo.user(), KUser().loginName());
}

void KProcessListTest::testProcessInfoNotFound()
{
    KProcessList::KProcessInfo processInfo = KProcessList::processInfo(-1);
    QVERIFY(processInfo.isValid() == false);
}
