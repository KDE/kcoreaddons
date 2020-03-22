/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kjobtrackerinterface.h"

#include "kjob.h"

class Q_DECL_HIDDEN KJobTrackerInterface::Private
{
public:
    Private(KJobTrackerInterface *interface) : q(interface)
    {

    }

    KJobTrackerInterface *const q;
};

KJobTrackerInterface::KJobTrackerInterface(QObject *parent)
    : QObject(parent), d(new Private(this))
{
    qRegisterMetaType<QPair<QString,QString>>();
}

KJobTrackerInterface::~KJobTrackerInterface()
{
    delete d;
}

void KJobTrackerInterface::registerJob(KJob *job)
{
    QObject::connect(job, SIGNAL(finished(KJob*)),
                     this, SLOT(unregisterJob(KJob*)));
    QObject::connect(job, SIGNAL(finished(KJob*)),
                     this, SLOT(finished(KJob*)));

    QObject::connect(job, SIGNAL(suspended(KJob*)),
                     this, SLOT(suspended(KJob*)));
    QObject::connect(job, SIGNAL(resumed(KJob*)),
                     this, SLOT(resumed(KJob*)));

    QObject::connect(job, SIGNAL(description(KJob *, const QString &,
                                 const QPair<QString, QString> &,
                                 const QPair<QString, QString> &)),
                     this, SLOT(description(KJob *, const QString &,
                                            const QPair<QString, QString> &,
                                            const QPair<QString, QString> &)));
    QObject::connect(job, SIGNAL(infoMessage(KJob*,QString,QString)),
                     this, SLOT(infoMessage(KJob*,QString,QString)));
    QObject::connect(job, SIGNAL(warning(KJob*,QString,QString)),
                     this, SLOT(warning(KJob*,QString,QString)));

    QObject::connect(job, SIGNAL(totalAmount(KJob*,KJob::Unit,qulonglong)),
                     this, SLOT(totalAmount(KJob*,KJob::Unit,qulonglong)));
    QObject::connect(job, SIGNAL(processedAmount(KJob*,KJob::Unit,qulonglong)),
                     this, SLOT(processedAmount(KJob*,KJob::Unit,qulonglong)));
    QObject::connect(job, SIGNAL(percent(KJob*,ulong)),
                     this, SLOT(percent(KJob*,ulong)));
    QObject::connect(job, SIGNAL(speed(KJob*,ulong)),
                     this, SLOT(speed(KJob*,ulong)));
}

void KJobTrackerInterface::unregisterJob(KJob *job)
{
    job->disconnect(this);
}

void KJobTrackerInterface::finished(KJob *job)
{
    Q_UNUSED(job)
}

void KJobTrackerInterface::suspended(KJob *job)
{
    Q_UNUSED(job)
}

void KJobTrackerInterface::resumed(KJob *job)
{
    Q_UNUSED(job)
}

void KJobTrackerInterface::description(KJob *job, const QString &title,
                                       const QPair<QString, QString> &field1,
                                       const QPair<QString, QString> &field2)
{
    Q_UNUSED(job)
    Q_UNUSED(title)
    Q_UNUSED(field1)
    Q_UNUSED(field2)

}

void KJobTrackerInterface::infoMessage(KJob *job, const QString &plain, const QString &rich)
{
    Q_UNUSED(job)
    Q_UNUSED(plain)
    Q_UNUSED(rich)
}

void KJobTrackerInterface::warning(KJob *job, const QString &plain, const QString &rich)
{
    Q_UNUSED(job)
    Q_UNUSED(plain)
    Q_UNUSED(rich)
}

void KJobTrackerInterface::totalAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    Q_UNUSED(job)
    Q_UNUSED(unit)
    Q_UNUSED(amount)
}

void KJobTrackerInterface::processedAmount(KJob *job, KJob::Unit unit, qulonglong amount)
{
    Q_UNUSED(job)
    Q_UNUSED(unit)
    Q_UNUSED(amount)
}

void KJobTrackerInterface::percent(KJob *job, unsigned long percent)
{
    Q_UNUSED(job)
    Q_UNUSED(percent)
}

void KJobTrackerInterface::speed(KJob *job, unsigned long value)
{
    Q_UNUSED(job)
    Q_UNUSED(value)
}

#include "moc_kjobtrackerinterface.cpp"
