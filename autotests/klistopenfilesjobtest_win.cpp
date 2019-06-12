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

#include "klistopenfilesjobtest_win.h"
#include "klistopenfilesjob.h"
#include <QCoreApplication>
#include <QStringLiteral>
#include <QTest>

QTEST_MAIN(KListOpenFilesJobTest)

void KListOpenFilesJobTest::testNotSupported()
{
    QDir path(QCoreApplication::applicationDirPath());
    auto job = new KListOpenFilesJob(path.path());
    job->exec();
    QCOMPARE(job->error(), static_cast<int>(KListOpenFilesJob::Error::NotSupported));
    QCOMPARE(job->errorText(), QStringLiteral("KListOpenFilesJob is not supported on Windows"));
    QVERIFY(job->processInfoList().empty());
}
