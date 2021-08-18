/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2010 Jacopo De Simoi <wilderkde@gmail.com>
    SPDX-FileCopyrightText: 2014 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "klistopenfilesjob.h"
#include <QDir>
#include <QProcess>
#include <QRegularExpression>
#include <QVector>

class KListOpenFilesJobPrivate
{
public:
    KListOpenFilesJobPrivate(KListOpenFilesJob *Job, const QDir &Path)
        : job(Job)
        , path(Path)
    {
        QObject::connect(&lsofProcess, &QProcess::errorOccurred, job, [this](QProcess::ProcessError error) {
            lsofError(error);
        });

        QObject::connect(&lsofProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), job, [this](int exitCode, QProcess::ExitStatus exitStatus) {
            lsofFinished(exitCode, exitStatus);
        });
    }
    void start()
    {
        if (!path.exists()) {
            emitResult(static_cast<int>(KListOpenFilesJob::Error::DoesNotExist), QObject::tr("Path %1 doesn't exist").arg(path.path()));
            return;
        }
        lsofProcess.start(QStringLiteral("lsof"), {QStringLiteral("-t"), QStringLiteral("+d"), path.path()});
    }
    KProcessList::KProcessInfoList getProcessInfoList() const
    {
        return processInfoList;
    }

private:
    void lsofError(QProcess::ProcessError processError)
    {
        emitResult(static_cast<int>(KListOpenFilesJob::Error::InternalError), QObject::tr("Failed to execute `lsof' error code %1").arg(processError));
    }
    void lsofFinished(int, QProcess::ExitStatus)
    {
        if (hasEmittedResult) {
            return;
        }
        const QString out(QString::fromLocal8Bit(lsofProcess.readAll()));

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QVector<QStringView> pidList = QStringView(out).split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
#else
        const QVector<QStringRef> pidList = out.splitRef(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
#endif

        for (const auto &pidStr : pidList) {
            qint64 pid = pidStr.toLongLong();
            if (!pid) {
                continue;
            }
            processInfoList << KProcessList::processInfo(pid);
        }
        job->emitResult();
    }
    void emitResult(int error, const QString &errorText)
    {
        if (hasEmittedResult) {
            return;
        }
        job->setError(error);
        job->setErrorText(errorText);
        job->emitResult();
        hasEmittedResult = true;
    }

private:
    KListOpenFilesJob *job;
    const QDir path;
    bool hasEmittedResult = false;
    QProcess lsofProcess;
    KProcessList::KProcessInfoList processInfoList;
};

KListOpenFilesJob::KListOpenFilesJob(const QString &path)
    : d(new KListOpenFilesJobPrivate(this, path))
{
}

KListOpenFilesJob::~KListOpenFilesJob() = default;

void KListOpenFilesJob::start()
{
    d->start();
}

KProcessList::KProcessInfoList KListOpenFilesJob::processInfoList() const
{
    return d->getProcessInfoList();
}

#include "moc_klistopenfilesjob.cpp"
