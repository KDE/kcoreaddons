/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPOSITEJOB_H
#define KCOMPOSITEJOB_H

#include <kcoreaddons_export.h>
#include <kjob.h>

#include <QList>

class KCompositeJobPrivate;
/*!
 * \class KCompositeJob
 * \inmodule KCoreAddons
 *
 * \brief The base class for all jobs able to be composed of one
 * or more subjobs.
 *
 * \deprecated [6.15] in favor of a new replacement class KCompoundJob.
 *
 * Differences between the two classes to be aware of while porting to KCompoundJob:
 * 1. The most significant difference between KCompositeJob and KCompoundJob is
 *    that the former connects a virtual subjob-finished slot to the result() and
 *    the latter - to the finished() signal of each subjob. Thus, unlike KCompositeJob,
 *    KCompoundJob also removes subjobs that are killed quietly or destroyed.
 *    This difference between the two classes causes the following possibly breaking change.
 *    If a KJob emits the result() signal, it always does so right after emitting the
 *    finished() signal. Therefore, KCompositeJob always handles a subjob's result after all
 *    slots connected to the subjob's finished() signal return. In contrast, KCompoundJob's
 *    handling of a finishing subjob can occur in between the invocations of the slots connected
 *    to the subjob's finished() signal. Therefore, porting code that depends on relative order
 *    of a composite job's and its subjobs' finished() and result() signals can break things.
 * 2. Two protected virtual slots are renamed in KCompoundJob:
 *    slotResult() to subjobFinished() and slotInfoMessage() to subjobInfoMessage().
 * 3. The protected function clearSubjobs() is virtual in KCompoundJob.
 * 4. Unlike KCompositeJob::slotResult(), KCompoundJob::subjobFinished() does not acquire the
 *    error code and text of a subjob that finishes after the compound job itself finishes.
 */
class KCOREADDONS_EXPORT KCompositeJob : public KJob
{
    Q_OBJECT

public:
#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(6, 15)
    /*!
     * Creates a new KCompositeJob object.
     *
     * \a parent the parent QObject
     */
    KCOREADDONS_DEPRECATED_VERSION(6, 15, "Use KCompoundJob instead")
    explicit KCompositeJob(QObject *parent = nullptr);
#endif

    ~KCompositeJob() override;

protected:
    /*!
     * Add a job that has to be finished before a result
     * is emitted. This has obviously to be called before
     * the result has been emitted by the job.
     *
     * Note that the composite job takes ownership of \a job
     *
     * \a job the subjob to add
     *
     * Returns \c true if the job has been added correctly, false otherwise
     */
    virtual bool addSubjob(KJob *job);

    /*!
     * Mark a sub job as being done.
     *
     * The ownership of \a job is passed on to the caller.
     *
     * \a job the subjob to remove
     *
     * Returns \c true if the job has been removed correctly, false otherwise
     */
    virtual bool removeSubjob(KJob *job);

    /*!
     * Checks if this job has subjobs running.
     *
     * Returns \c true if we still have subjobs running, false otherwise
     */
    bool hasSubjobs() const;

    /*!
     * Returns the full list of sub jobs
     */
    const QList<KJob *> &subjobs() const;

    /*!
     * Clears the list of subjobs.
     *
     * Note that this will *not* delete the subjobs.
     * Ownership of the subjobs is passed on to the caller.
     */
    void clearSubjobs();

protected Q_SLOTS:
    /*!
     * Called whenever a subjob finishes.
     *
     * Default implementation checks for errors and propagates
     * to parent job, and in all cases it calls removeSubjob.
     *
     * \a job the subjob
     */
    virtual void slotResult(KJob *job);

    /*!
     * Forward signal from subjob.
     *
     * \a job the subjob
     *
     * \a message the info message
     *
     * \sa infoMessage()
     */
    virtual void slotInfoMessage(KJob *job, const QString &message);

protected:
#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(6, 15)
    KCOREADDONS_DEPRECATED_VERSION(6, 15, "Use KCompoundJob instead")
    KCOREADDONS_NO_EXPORT KCompositeJob(KCompositeJobPrivate &dd, QObject *parent);
#endif

private:
    Q_DECLARE_PRIVATE(KCompositeJob)
};

#endif
