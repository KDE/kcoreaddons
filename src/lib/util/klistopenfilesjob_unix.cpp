/*
 *  This file is part of the KDE project
 *  Copyright (C) 2010 by Jacopo De Simoi <wilderkde@gmail.com>
 *  Copyright (C) 2014 by Lukáš Tinkl <ltinkl@redhat.com>
 *  Copyright (C) 2016 by Kai Uwe Broulik <kde@privat.broulik.de>
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

#include "klistopenfilesjob.h"
#include <QDir>
#include <QProcess>

class KListOpenFilesJobPrivate
{
public:
    KListOpenFilesJobPrivate(KListOpenFilesJob *Job, const QDir &Path)
        : job(Job)
        , path(Path)
        , hasEmittedResult(false)
    {
        QObject::connect(&lsofProcess, &QProcess::errorOccurred, [this](QProcess::ProcessError error) {
            lsofError(error);
        });
        QObject::connect(&lsofProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                         [this](int exitCode, QProcess::ExitStatus exitStatus) {
            lsofFinished(exitCode, exitStatus);
        });
    }
    void start()
    {
        if (!path.exists()) {
            emitResult(static_cast<int>(KListOpenFilesJob::Error::DoesNotExist),
                       QObject::tr("Path %1 doesn't exist").arg(path.path()));
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
        emitResult(static_cast<int>(KListOpenFilesJob::Error::InternalError),
                   QObject::tr("Failed to execute `lsof' error code %1").arg(processError));
    }
    void lsofFinished(int, QProcess::ExitStatus)
    {
        if (hasEmittedResult) {
            return;
        }
        QStringList blockApps;
        const QString out(QString::fromLocal8Bit(lsofProcess.readAll()));
        QStringList pidList = out.split(QRegExp(QStringLiteral("\\s+")), QString::SkipEmptyParts);
        pidList.removeDuplicates();
        for (const auto& pidStr : qAsConst(pidList)) {
            qint64 pid = pidStr.toLongLong();
            if (!pid) {
                continue;
            }
            processInfoList << KProcessList::processInfo(pid);
        }
        job->emitResult();
    }
    void emitResult(int error, const QString& errorText)
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
    bool hasEmittedResult;
    QProcess lsofProcess;
    KProcessList::KProcessInfoList processInfoList;
};

KListOpenFilesJob::KListOpenFilesJob(const QString& path)
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
