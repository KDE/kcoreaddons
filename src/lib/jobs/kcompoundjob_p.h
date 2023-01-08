/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPOUNDJOB_P_H
#define KCOMPOUNDJOB_P_H

#include "kcompoundjob.h"

#include "kjob_p.h"

// This is a private class, but it's exported for KIO::Job's
// usage. Other Job classes in KDE Frameworks may use it too.
class KCOREADDONS_EXPORT KCompoundJobPrivate : public KJobPrivate
{
public:
    KCompoundJobPrivate();
    ~KCompoundJobPrivate() override;

    virtual void disconnectSubjob(KJob *job);

    QList<KJob *> m_subjobs;

    Q_DECLARE_PUBLIC(KCompoundJob)
};

#endif
