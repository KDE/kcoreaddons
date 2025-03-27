/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPOUNDJOB_P_H
#define KCOMPOUNDJOB_P_H

#include "kcompoundjob.h"

#include "kjob_p.h"

class KCompoundJobPrivate : public KJobPrivate
{
public:
    KCompoundJobPrivate();
    ~KCompoundJobPrivate() override;

    virtual void disconnectSubjob(KJob *job);

    QList<KJob *> m_subjobs;

    Q_DECLARE_PUBLIC(KCompoundJob)
};

#endif
