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

KJobTrackerInterface::~KJobTrackerInterface() = default;

void KJobTrackerInterface::registerJob(KJob *job)
{
    connect(job, &KJob::finished, this, &KJobTrackerInterface::unregisterJob);
    connect(job, &KJob::finished, this, &KJobTrackerInterface::finished);
    connect(job, &KJob::suspended, this, &KJobTrackerInterface::suspended);
    connect(job, &KJob::resumed,this, &KJobTrackerInterface::resumed);

    connect(job, &KJob::description, this, &KJobTrackerInterface::description);
    connect(job, &KJob::infoMessage, this, &KJobTrackerInterface::infoMessage);
    connect(job, &KJob::warning, this, &KJobTrackerInterface::warning);

    connect(job, QOverload<KJob *, KJob::Unit, qulonglong>::of(&KJob::totalAmount),
            this, &KJobTrackerInterface::totalAmount);
    connect(job, QOverload<KJob *, KJob::Unit, qulonglong>::of(&KJob::processedAmount),
            this, &KJobTrackerInterface::processedAmount);
    connect(job, QOverload<KJob *, unsigned long>::of(&KJob::percent),
            this, &KJobTrackerInterface::percent);
    connect(job, &KJob::speed, this, &KJobTrackerInterface::speed);
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
