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

#include "klistopenfilesjobtest_unix.h"
#include "klistopenfilesjob.h"
#include <QCoreApplication>
#include <QStringLiteral>
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>

QTEST_MAIN(KListOpenFilesJobTest)

void KListOpenFilesJobTest::testOpenFiles()
{
    QDir path(QCoreApplication::applicationDirPath());
    auto job = new KListOpenFilesJob(path.path());
    job->exec();
    QCOMPARE(job->error(), KJob::NoError);
    auto processInfoList = job->processInfoList();
    QVERIFY(!processInfoList.empty());
    auto testProcessIterator = std::find_if(processInfoList.begin(), processInfoList.end(),
                                            [](const KProcessList::KProcessInfo& info)
    {
        return info.pid() == QCoreApplication::applicationPid();
    });
    QVERIFY(testProcessIterator != processInfoList.end());
    const auto& processInfo = *testProcessIterator;
    QVERIFY(processInfo.isValid());
    QCOMPARE(processInfo.pid(), QCoreApplication::applicationPid());
}

void KListOpenFilesJobTest::testNoOpenFiles()
{
    QTemporaryDir tempDir;
    auto job = new KListOpenFilesJob(tempDir.path());
    job->exec();
    QCOMPARE(job->error(), KJob::NoError);
    QVERIFY(job->processInfoList().empty());
}

void KListOpenFilesJobTest::testNonExistingDir()
{
    QString nonExistingDir(QStringLiteral("/does/not/exist"));
    auto job = new KListOpenFilesJob(nonExistingDir);
    job->exec();
    QCOMPARE(job->error(), static_cast<int>(KListOpenFilesJob::Error::DoesNotExist));
    QCOMPARE(job->errorText(), QStringLiteral("Path %1 doesn't exist").arg(nonExistingDir));
    QVERIFY(job->processInfoList().empty());
}

/**
 * @brief Helper class to temporarily set an environment variable and reset it on destruction
 */
class ScopedEnvVariable
{
public:
    ScopedEnvVariable(const QLatin1String& Name, const QByteArray& NewValue)
        : name(Name)
        , originalValue(qgetenv(name.latin1()))
    {
        qputenv(name.latin1(), NewValue);
    }
    ~ScopedEnvVariable()
    {
        qputenv(name.latin1(), originalValue);
    }
private:
    const QLatin1String name;
    const QByteArray originalValue;
};

void KListOpenFilesJobTest::testLsofNotFound()
{
    // This test relies on clearing the PATH variable so that lsof is not found
    ScopedEnvVariable emptyPathEnvironment(QLatin1String("PATH"), QByteArray());
    QDir path(QCoreApplication::applicationDirPath());
    auto job = new KListOpenFilesJob(path.path());
    job->exec();
    QCOMPARE(job->error(), static_cast<int>(KListOpenFilesJob::Error::InternalError));
    QVERIFY(job->processInfoList().empty());
}
