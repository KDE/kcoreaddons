/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Kevin Funk <kevin@kfunk.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcompositejobtest.h"

#include <QEventLoop>
#include <QMetaEnum>
#include <QTest>
#include <QSignalSpy>
#include <QTimer>

#include <string>

namespace {

struct JobSpies {
    QSignalSpy finished;
    QSignalSpy result;
    QSignalSpy destroyed;
    explicit JobSpies(KJob *job)
        : finished(job, &KJob::finished)
        , result(job, &KJob::result)
        , destroyed(job, &QObject::destroyed)
    {}
};

}

TestJob::TestJob(QObject *parent)
    : KJob(parent)
{
}

void TestJob::start()
{
    QTimer::singleShot(1000, this, &TestJob::emitResult);
}

KillableTestJob::KillableTestJob(QObject *parent)
    : TestJob(parent)
{
    setCapabilities(Killable);
}

bool KillableTestJob::doKill()
{
    return true;
}

void CompositeJob::start()
{
    if (hasSubjobs()) {
        subjobs().first()->start();
    } else {
        emitResult();
    }
}

void CompositeJob::slotResult(KJob *job)
{
    KCompositeJob::slotResult(job);

    if (error()) {
        return; // KCompositeJob::slotResult() must have called emitResult().
    }
    if (hasSubjobs()) {
        // start next
        subjobs().first()->start();
    } else {
        emitResult();
    }
}

KCompositeJobTest::KCompositeJobTest()
{
}

/**
 * In case a composite job is deleted during execution
 * we still want to assure that we don't crash
 *
 * see bug: https://bugs.kde.org/show_bug.cgi?id=230692
 */
void KCompositeJobTest::testDeletionDuringExecution()
{
    QObject *someParent = new QObject;
    KJob *job = new TestJob(someParent);

    CompositeJob *compositeJob = new CompositeJob;
    compositeJob->setAutoDelete(false);
    QVERIFY(compositeJob->addSubjob(job));

    QCOMPARE(job->parent(), compositeJob);

    QSignalSpy destroyed_spy(job, SIGNAL(destroyed(QObject*)));
    // check if job got reparented properly
    delete someParent; someParent = nullptr;
    // the job should still exist, because it is a child of KCompositeJob now
    QCOMPARE(destroyed_spy.size(), 0);

    // start async, the subjob takes 1 second to finish
    compositeJob->start();

    // delete the job during the execution
    delete compositeJob; compositeJob = nullptr;
    // at this point, the subjob should be deleted, too
    QCOMPARE(destroyed_spy.size(), 1);
}

void KCompositeJobTest::testFinishingSubjob_data()
{
    QTest::addColumn<Action>("action");
    QTest::addColumn<bool>("crashOnFailure");

    const auto actionName = [](Action action) {
        return QMetaEnum::fromType<Action>().valueToKey(static_cast<int>(action));
    };

    for (bool crashOnFailure : {false, true}) {
        for (Action action : {Action::Finish, Action::KillVerbosely,
                              Action::KillQuietly, Action::Destroy}) {
            const auto dataTag = std::string{actionName(action)} + "-a-subjob-"
                                    + (crashOnFailure ? "segfault-on-failure"
                                                      : "composite-job-destroyed");
            QTest::newRow(dataTag.c_str()) << action << crashOnFailure;
        }
    }
}

void KCompositeJobTest::testFinishingSubjob()
{
    auto *job = new KillableTestJob;
    CompositeJob *compositeJob = new CompositeJob;
    QVERIFY(compositeJob->addSubjob(job));

    JobSpies jobSpies(job);
    JobSpies compositeJobSpies(compositeJob);

    compositeJob->start();

    QFETCH(Action, action);
    switch (action) {
        case Action::Finish:
            job->emitResult();
            break;
        case Action::KillVerbosely:
            job->kill(KJob::EmitResult);
            break;
        case Action::KillQuietly:
            job->kill(KJob::Quietly);
            break;
        case Action::Destroy:
            job->deleteLater();
            break;
    }

    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    connect(compositeJob, &QObject::destroyed, &loop, &QEventLoop::quit);
    loop.exec();

    // The following 3 comparisons verify that KJob works as expected.
    QCOMPARE(jobSpies.finished.size(), 1); // KJob::finished() is always emitted.
    // KJob::result() is not emitted when a job is killed quietly or destroyed.
    QCOMPARE(jobSpies.result.size(), action == Action::Finish
                                     || action == Action::KillVerbosely);
    // An auto-delete job is destroyed via deleteLater() when finished.
    QCOMPARE(jobSpies.destroyed.size(), 1);

    // KCompositeJob must listen to &KJob::finished signal to call slotResult()
    // no matter how a subjob is finished - normally, killed or destroyed.
    // CompositeJob calls emitResult() and is destroyed when its last subjob finishes.
    QFETCH(bool, crashOnFailure);
    if (crashOnFailure) {
        if (compositeJobSpies.destroyed.empty()) {
            // compositeJob is still alive. This must be a bug.
            // The clearSubjobs() call will segfault if the already destroyed job
            // has not been removed from the subjob list.
            compositeJob->clearSubjobs();
            delete compositeJob;
        }
    } else {
        QCOMPARE(compositeJobSpies.finished.size(), 1);
        QCOMPARE(compositeJobSpies.result.size(), 1);
        QCOMPARE(compositeJobSpies.destroyed.size(), 1);
    }
}

QTEST_GUILESS_MAIN(KCompositeJobTest)

