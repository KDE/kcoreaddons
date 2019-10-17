/**************************************************************************
**
** This file is part of the KDE Frameworks
**
** Copyright (c) 2019 Tobias C. Berner <tcberner@FreeBSD.org>
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
**************************************************************************/

#include "kprocesslist.h"
#include "kprocesslist_unix_procstat_p.h"

#include <QProcess>
#include <QDir>


using namespace KProcessList;

// Determine UNIX processes by using the procstat library
KProcessInfoList KProcessList::processInfoList()
{
    KProcessInfoList rc;

    ProcStat pstat;
    if (!pstat)
    {
        return rc;
    }

    ProcStatProcesses procs(pstat);
    for (const auto& process_info: procs)
    {
        rc.push_back(process_info);
    }

    return rc;
}
