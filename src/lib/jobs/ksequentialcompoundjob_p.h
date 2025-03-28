/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007-2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSEQUENTIALCOMPOUNDJOB_P_H
#define KSEQUENTIALCOMPOUNDJOB_P_H

#include "ksequentialcompoundjob.h"

#include "kcompoundjob_p.h"

class KSequentialCompoundJobPrivate : public KCompoundJobPrivate
{
public:
    KSequentialCompoundJobPrivate();
    ~KSequentialCompoundJobPrivate() override;

    bool isCurrentlyRunningSubjob(KJob *job) const;

    /*!
     * Start \a m_subjobs.front().
     *
     * \a m_subjobs should not be empty
     */
    void startNextSubjob();

    void disconnectSubjob(KJob *job) override;

    bool m_abortOnSubjobError = true;

    bool m_killingSubjob = false;
    bool m_killingFailed = false;

    int m_jobIndex = -1;
    int m_jobCount = 0;

    Q_DECLARE_PUBLIC(KSequentialCompoundJob)
};

#endif
