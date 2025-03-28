/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSIMPLESEQUENTIALCOMPOUNDJOB_H
#define KSIMPLESEQUENTIALCOMPOUNDJOB_H

#include <kcoreaddons_export.h>
#include <ksequentialcompoundjob.h>

/*!
 * \class KSimpleSequentialCompoundJob ksimplesequentialcompoundjob.h KSimpleSequentialCompoundJob
 *
 * A sequential compound job with \c public addSubjob().
 * \sa KSequentialCompoundJob
 *
 * \since 6.15
 */
class KCOREADDONS_EXPORT KSimpleSequentialCompoundJob : public KSequentialCompoundJob
{
    Q_OBJECT
public:
    using KSequentialCompoundJob::addSubjob;
    using KSequentialCompoundJob::KSequentialCompoundJob;
};

#endif
