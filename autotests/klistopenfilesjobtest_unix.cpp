/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "klistopenfilesjobtest_unix.h"
#include "klistopenfilesjob.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QStringLiteral>
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>

QTEST_MAIN(KListOpenFilesJobTest)
void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

namespace
{
bool hasLsofInstalled()
{
    return !QStandardPaths::findExecutable(QStringLiteral("lsof")).isEmpty();
}

}

void KListOpenFilesJobTest::testOpenFiles()
{
    if (!hasLsofInstalled()) {
        QSKIP("lsof is not installed - skipping test");
    }
    QTemporaryDir tempDir;
    QFile tempFile(tempDir.path() + QStringLiteral("/file"));
    QVERIFY(tempFile.open(QIODevice::WriteOnly));
    auto job = new KListOpenFilesJob(tempDir.path());
    QVERIFY2(job->exec(), qPrintable(job->errorString()));
    QCOMPARE(job->error(), KJob::NoError);
    auto processInfoList = job->processInfoList();
    QVERIFY(!processInfoList.empty());
    auto testProcessIterator = std::find_if(processInfoList.begin(), processInfoList.end(), [](const KProcessList::KProcessInfo &info) {
        return info.pid() == QCoreApplication::applicationPid();
    });
    QVERIFY(testProcessIterator != processInfoList.end());
    const auto &processInfo = *testProcessIterator;
    QVERIFY(processInfo.isValid());
    QCOMPARE(processInfo.pid(), QCoreApplication::applicationPid());
}

void KListOpenFilesJobTest::testNoOpenFiles()
{
    if (!hasLsofInstalled()) {
        QSKIP("lsof is not installed - skipping test");
    }
    QTemporaryDir tempDir;
    auto job = new KListOpenFilesJob(tempDir.path());
    QVERIFY2(job->exec(), qPrintable(job->errorString()));
    QCOMPARE(job->error(), KJob::NoError);
    QVERIFY(job->processInfoList().empty());
}

void KListOpenFilesJobTest::testNonExistingDir()
{
    if (!hasLsofInstalled()) {
        QSKIP("lsof is not installed - skipping test");
    }
    QString nonExistingDir(QStringLiteral("/does/not/exist"));
    auto job = new KListOpenFilesJob(nonExistingDir);
    QVERIFY(!job->exec());
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
    ScopedEnvVariable(const QLatin1String &Name, const QByteArray &NewValue)
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
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), static_cast<int>(KListOpenFilesJob::Error::InternalError));
    QVERIFY(job->processInfoList().empty());
}
