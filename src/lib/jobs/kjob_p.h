/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KJOB_P_H
#define KJOB_P_H

#include "kjob.h"
#include <QMap>
#include <QEventLoopLocker>

class KJobUiDelegate;
class QTimer;
class QEventLoop;

// This is a private class, but it's exported for
// KIO::Job's usage. Other Job classes in kdelibs may
// use it too.
class KCOREADDONS_EXPORT KJobPrivate
{
public:
    KJobPrivate();
    virtual ~KJobPrivate();

    KJob *q_ptr = nullptr;

    KJobUiDelegate *uiDelegate = nullptr;
    QString errorText;
    int error = KJob::NoError;
    KJob::Unit progressUnit = KJob::Bytes;
    QMap<KJob::Unit, qulonglong> processedAmount;
    QMap<KJob::Unit, qulonglong> totalAmount;
    unsigned long percentage = 0;
    QTimer *speedTimer = nullptr;
    QEventLoop *eventLoop = nullptr;
    // eventLoopLocker prevents QCoreApplication from exiting when the last
    // window is closed until the job has finished running
    QEventLoopLocker eventLoopLocker;
    KJob::Capabilities capabilities = KJob::NoCapabilities;
    bool suspended = false;
    bool isAutoDelete = true;

    void _k_speedTimeout();

    bool isFinished = false;

    Q_DECLARE_PUBLIC(KJob)
};

#endif
