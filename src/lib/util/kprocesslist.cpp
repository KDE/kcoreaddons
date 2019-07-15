/**************************************************************************
**
** This file is part of the KDE Frameworks
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (c) 2019 David Hallas <david@davidhallas.dk>
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at info@qt.nokia.com.
**
**************************************************************************/

#include "kprocesslist.h"
#include "kprocesslist_p.h"
#include <algorithm>

using namespace KProcessList;

KProcessInfoPrivate::KProcessInfoPrivate() :
    valid(false),
    pid(-1)
{
}

KProcessInfo::KProcessInfo() :
    d_ptr(new KProcessInfoPrivate)
{
}

KProcessInfo::KProcessInfo(qint64 pid, const QString& command, const QString& user) :
    KProcessInfo(pid, command, command, user)
{}


KProcessInfo::KProcessInfo(qint64 pid, const QString& command, const QString &name, const QString& user) :
    d_ptr(new KProcessInfoPrivate)
{
    d_ptr->valid = true;
    d_ptr->pid = pid;
    d_ptr->name = name;
    d_ptr->command = command;
    d_ptr->user = user;
}

KProcessInfo::KProcessInfo(const KProcessInfo &other) :
    d_ptr(new KProcessInfoPrivate)
{
    *this = other;
}

KProcessInfo::~KProcessInfo()
{
}

KProcessInfo &KProcessInfo::operator=(const KProcessInfo &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

bool KProcessInfo::isValid() const
{
    return d_ptr->valid;
}

qint64 KProcessInfo::pid() const
{
    return d_ptr->pid;
}

QString KProcessInfo::name() const
{
    return d_ptr->name;
}

QString KProcessInfo::command() const
{
    return d_ptr->command;
}

QString KProcessInfo::user() const
{
    return d_ptr->user;
}

KProcessInfo KProcessList::processInfo(qint64 pid)
{
    KProcessInfoList processInfoList = KProcessList::processInfoList();
    auto testProcessIterator = std::find_if(processInfoList.begin(), processInfoList.end(),
                                            [pid](const KProcessList::KProcessInfo& info)
    {
        return info.pid() == pid;
    });
    if (testProcessIterator != processInfoList.end()) {
        return *testProcessIterator;
    }
    return KProcessInfo();
}
