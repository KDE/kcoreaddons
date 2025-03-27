/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ksequentialcompoundjob.h"
#include "ksequentialcompoundjob_p.h"

#include "kcoreaddons_debug.h"

KSequentialCompoundJobPrivate::KSequentialCompoundJobPrivate() = default;
KSequentialCompoundJobPrivate::~KSequentialCompoundJobPrivate() = default;

bool KSequentialCompoundJobPrivate::isCurrentlyRunningSubjob(KJob *job) const
{
    return m_jobIndex >= 0 && !m_subjobs.empty() && job == m_subjobs.constFirst();
}

void KSequentialCompoundJobPrivate::startNextSubjob()
{
    ++m_jobIndex;
    Q_ASSERT(!m_subjobs.empty());
    auto *const job = m_subjobs.constFirst();

    qCDebug(KCOREADDONS_DEBUG) << "starting subjob" << m_jobIndex + 1 << "of" << m_jobCount << ':' << job;
    job->start();
}

void KSequentialCompoundJobPrivate::disconnectSubjob(KJob *job)
{
    Q_Q(KSequentialCompoundJob);
    QObject::disconnect(job, &KJob::percentChanged, q, &KSequentialCompoundJob::subjobPercentChanged);
    KCompoundJobPrivate::disconnectSubjob(job);
}

KSequentialCompoundJob::KSequentialCompoundJob(QObject *parent)
    : KSequentialCompoundJob(*new KSequentialCompoundJobPrivate, parent)
{
}

KSequentialCompoundJob::KSequentialCompoundJob(KSequentialCompoundJobPrivate &dd, QObject *parent)
    : KCompoundJob(dd, parent)
{
    setCapabilities(Killable);
}

KSequentialCompoundJob::~KSequentialCompoundJob() = default;

void KSequentialCompoundJob::setAbortOnSubjobError(bool abort)
{
    Q_D(KSequentialCompoundJob);
    d->m_abortOnSubjobError = abort;
}

void KSequentialCompoundJob::start()
{
    Q_D(KSequentialCompoundJob);
    if (d->m_subjobs.empty()) {
        qCDebug(KCOREADDONS_DEBUG) << "no subjobs, finishing in start()";
        emitResult();
        return;
    }

    d->startNextSubjob();
}

void KSequentialCompoundJob::subjobPercentChanged(KJob *job, unsigned long percent)
{
    Q_D(KSequentialCompoundJob);
    Q_ASSERT(d->m_jobIndex < d->m_jobCount); // invariant
    if (!d->isCurrentlyRunningSubjob(job)) {
        qCDebug(KCOREADDONS_DEBUG) << "ignoring percentChanged() signal emitted by an unstarted or finished subjob" << job;
        return;
    }
    Q_ASSERT(d->m_jobIndex >= 0);

    const unsigned long totalPercent = (100.0 * d->m_jobIndex + percent) / d->m_jobCount;
    qCDebug(KCOREADDONS_DEBUG) << "subjob percent:" << percent << "; total percent:" << totalPercent;
    setPercent(totalPercent);
}

void KSequentialCompoundJob::subjobFinished(KJob *job)
{
    Q_D(KSequentialCompoundJob);
    if (d->m_killingSubjob || isFinished()) {
        // doKill() will return true and this compound job will finish, or already finished
        removeSubjob(job);
        return;
    }

    Q_ASSERT(d->m_jobIndex < d->m_jobCount); // invariant
    // Note: isCurrentlyRunningSubjob(job) must be checked before calling removeSubjob(job).
    if (!d->isCurrentlyRunningSubjob(job)) {
        qCDebug(KCOREADDONS_DEBUG) << "unstarted subjob finished:" << job;
        removeSubjob(job);
        return;
    }

    const bool registeredSubjob = removeSubjob(job);
    Q_ASSERT(registeredSubjob); // because isCurrentlyRunningSubjob(job) returned true

    Q_ASSERT(d->m_jobIndex >= 0); // because isCurrentlyRunningSubjob(job) returned true
    const unsigned long totalPercent = 100.0 * (d->m_jobIndex + 1) / d->m_jobCount;
    qCDebug(KCOREADDONS_DEBUG) << "subjob finished:" << job << "; total percent:" << totalPercent;
    setPercent(totalPercent);

    int error = job->error();
    if (!error && d->m_killingFailed) {
        error = KilledJobError;
    }

    // Abort if job is the subjob we failed to kill and in case of error.
    const bool abort = d->m_killingFailed || (d->m_abortOnSubjobError && error);
    if (abort) {
        qCDebug(KCOREADDONS_DEBUG) << "aborting on subjob error:" << error << job->errorText();
    }

    // Finish in order to abort, or if all subjobs have finished. Propagate the last-run subjob's error.
    if (abort || d->m_subjobs.empty()) {
        setError(error);
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    qCDebug(KCOREADDONS_DEBUG) << "remaining subjobs:" << d->m_subjobs;
    d->startNextSubjob();
}

bool KSequentialCompoundJob::addSubjob(KJob *job)
{
    Q_D(KSequentialCompoundJob);
    if (!KCompoundJob::addSubjob(job)) {
        return false;
    }
    ++d->m_jobCount;
    connect(job, &KJob::percentChanged, this, &KSequentialCompoundJob::subjobPercentChanged);
    return true;
}

bool KSequentialCompoundJob::doKill()
{
    Q_D(KSequentialCompoundJob);
    // Don't check isFinished() here, because KJob::kill() calls doKill() only if the job has not finished.
    if (d->m_killingSubjob) {
        qCDebug(KCOREADDONS_DEBUG) << "killing sequential compound job recursively fails";
        return false;
    }
    if (d->m_jobIndex == -1) {
        qCDebug(KCOREADDONS_DEBUG) << "killing unstarted sequential compound job";
        // Any unstarted subjobs will be deleted along with this compound job, which is their parent.
        return true;
    }
    if (d->m_subjobs.empty()) {
        qCDebug(KCOREADDONS_DEBUG) << "killing sequential compound job with zero remaining subjobs";
        return true;
    }

    auto *const job = d->m_subjobs.constFirst();
    qCDebug(KCOREADDONS_DEBUG) << "killing running subjob" << job;

    d->m_killingSubjob = true;
    const bool killed = job->kill();
    d->m_killingSubjob = false;

    d->m_killingFailed = !killed;
    if (d->m_killingFailed) {
        qCDebug(KCOREADDONS_DEBUG) << "failed to kill subjob" << job;
        if (d->m_subjobs.empty() || d->m_subjobs.constFirst() != job) {
            qCDebug(KCOREADDONS_DEBUG) << "... but the subjob finished or was removed, assume killed. Remaining subjobs:" << d->m_subjobs;
            return true;
        }
    }

    return killed;
}

#include "moc_ksequentialcompoundjob.cpp"
#include "moc_ksimplesequentialcompoundjob.cpp"
