/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Kevin Funk <kevin@kfunk.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCOMPOSITEJOBTEST_H
#define KCOMPOSITEJOBTEST_H

#include <QObject>

#include "kcompositejob.h"

class TestJob : public KJob
{
    Q_OBJECT

public:
    explicit TestJob(QObject *parent = nullptr);

    /// Takes 1 second to finish
    void start() override;

    using KJob::emitResult;
};

class KillableTestJob : public TestJob
{
    Q_OBJECT

public:
    explicit KillableTestJob(QObject *parent = nullptr);

protected:
    bool doKill() override;
};

class CompositeJob : public KCompositeJob
{
    Q_OBJECT

public:
    explicit CompositeJob(QObject *parent = nullptr) : KCompositeJob(parent) {}

    void start() override;
    using KCompositeJob::addSubjob;
    using KCompositeJob::clearSubjobs;

protected Q_SLOTS:
    void slotResult(KJob *job) override;
};

class KCompositeJobTest : public QObject
{
    Q_OBJECT

public:
    enum class Action {
        Finish,
        KillVerbosely,
        KillQuietly,
        Destroy
    };
    Q_ENUM(Action)

    KCompositeJobTest();

private Q_SLOTS:
    void testDeletionDuringExecution();

    void testFinishingSubjob();
    void testFinishingSubjob_data();
};

#endif // KCOMPOSITEJOBTEST_H
