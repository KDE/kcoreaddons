/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPOUNDJOB_H
#define KCOMPOUNDJOB_H

#include <kcoreaddons_export.h>
#include <kjob.h>

#include <QList>

class KCompoundJobPrivate;
/*!
 * \class KCompoundJob kcompoundjob.h KCompoundJob
 *
 * The base class for all jobs able to be composed of one
 * or more subjobs.
 *
 * \since 6.15
 */
class KCOREADDONS_EXPORT KCompoundJob : public KJob
{
    Q_OBJECT

public:
    /*!
     * Creates a new KCompoundJob object.
     *
     * \a parent the parent QObject
     */
    explicit KCompoundJob(QObject *parent = nullptr);

    /*!
     * Destroys a KCompoundJob object.
     */
    ~KCompoundJob() override;

protected:
    /*!
     * Add a job that has to be finished before a result
     * is emitted. This has obviously to be called before
     * the finished() signal has been emitted by the job.
     *
     * Note that the compound job takes ownership of \a job
     * until it starts and finishes.
     *
     * \a job the subjob to add
     *
     * Returns whether the job has been added correctly
     */
    virtual bool addSubjob(KJob *job);

    /*!
     * Mark a sub job as being done.
     *
     * The ownership of \a job is passed on to the caller.
     *
     * \a job the subjob to remove
     *
     * Returns whether the job has been removed correctly
     */
    virtual bool removeSubjob(KJob *job);

    /*!
     * Returns whether this job has subjobs running
     */
    bool hasSubjobs() const;

    /*!
     * Returns the full list of subjobs
     */
    const QList<KJob *> &subjobs() const;

    /*!
     * Clear the list of subjobs.
     *
     * Note that this will *not* delete the subjobs.
     * Ownership of the subjobs is passed on to the caller.
     */
    virtual void clearSubjobs();

protected Q_SLOTS:
    /*!
     * This slot is invoked whenever a subjob finishes.
     *
     * The default implementation checks for errors and propagates
     * to the compound job, and in all cases calls removeSubjob().
     *
     * Returns job the subjob
     */
    virtual void subjobFinished(KJob *job);

    /*!
     * Forward signal from subjob.
     *
     * \a job the subjob
     * \a message the info message
     * \sa infoMessage()
     */
    virtual void subjobInfoMessage(KJob *job, const QString &message);

protected:
    KCOREADDONS_NO_EXPORT KCompoundJob(KCompoundJobPrivate &dd, QObject *parent);

private:
    Q_DECLARE_PRIVATE(KCompoundJob)
};

#endif
