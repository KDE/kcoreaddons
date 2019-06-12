/*
 *  This file is part of the KDE project
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
#include <QTimer>

class KListOpenFilesJobPrivate
{
};

KListOpenFilesJob::KListOpenFilesJob(const QString&)
    : d(nullptr)
{
}

KListOpenFilesJob::~KListOpenFilesJob() = default;

void KListOpenFilesJob::start()
{
    QTimer::singleShot(0, [this](){
        setError(static_cast<int>(KListOpenFilesJob::Error::NotSupported));
        setErrorText(QObject::tr("KListOpenFilesJob is not supported on Windows"));
        emitResult();
    });
}

KProcessList::KProcessInfoList KListOpenFilesJob::processInfoList() const
{
    return KProcessList::KProcessInfoList();
}

#include "moc_klistopenfilesjob.cpp"
