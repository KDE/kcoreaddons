/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KSEQUENTIALCOMPOUNDJOBTEST_H
#define KSEQUENTIALCOMPOUNDJOBTEST_H

#include <KJob>

#include <QObject>

class TestJob : public KJob
{
    Q_OBJECT
public:
    void start() override
    {
        Q_EMIT started(this);
    }

    void emitInfoMessage(const QString &info)
    {
        Q_EMIT infoMessage(this, info);
    }

    using KJob::emitResult;
    using KJob::setError;
    using KJob::setErrorText;
    using KJob::setPercent;

Q_SIGNALS:
    void started(KJob *job);
};

class KillableTestJob : public TestJob
{
    Q_OBJECT
public:
    explicit KillableTestJob()
    {
        setCapabilities(Killable);
    }

    void setKillingSucceeds(bool succeeds)
    {
        m_killingSucceeds = succeeds;
    }

Q_SIGNALS:
    void killed(bool successfully);

protected:
    bool doKill() override
    {
        Q_EMIT killed(m_killingSucceeds);
        return m_killingSucceeds;
    }

private:
    bool m_killingSucceeds = true;
};

class KSequentialCompoundJobTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void runZeroJobs();
    void runOneJob();
    void runTwoJobs();

    void addRemoveClearSubjob_data();
    void addRemoveClearSubjob();
    void addClearSubjobs();

    void subjobPercentChanged();

    void abortOnSubjobError();
    void disableAbortOnSubjobError_data();
    void disableAbortOnSubjobError();

    void finishWrongSubjob_data();
    void finishWrongSubjob();

    void killUnstartedCompoundJob_data();
    void killUnstartedCompoundJob();
    void killFinishedCompoundJob_data();
    void killFinishedCompoundJob();

    void killRunningCompoundJob();
    void killingSubjobFails();

    void killRunningCompoundJobRepeatedly_data();
    void killRunningCompoundJobRepeatedly();
};

#endif // KSEQUENTIALCOMPOUNDJOBTEST_H
