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
 * @since 6.14
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
     *
     * \a job the subjob to add
     *
     * Returns true if the job has been added correctly, false otherwise
     */
    virtual bool addSubjob(KJob *job);

    /*!
     * Mark a sub job as being done.
     *
     * The ownership of \a job is passed on to the caller.
     *
     * \a job the subjob to remove
     *
     * Returns true if the job has been removed correctly, false otherwise
     */
    virtual bool removeSubjob(KJob *job);

    /*!
     * Checks if this job has subjobs running.
     *
     * Returns true if we still have subjobs running, false otherwise
     */
    bool hasSubjobs() const;

    /*!
     * Retrieves the list of the subjobs.
     *
     * Returns the full list of sub jobs
     */
    const QList<KJob *> &subjobs() const;

    /*!
     * Clears the list of subjobs.
     *
     * Note that this will *not* delete the subjobs.
     * Ownership of the subjobs is passed on to the caller.
     */
    virtual void clearSubjobs();

protected Q_SLOTS:
    /*!
     * Called whenever a subjob finishes.
     * Default implementation checks for errors and propagates
     * to parent job, and in all cases it calls removeSubjob.
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
