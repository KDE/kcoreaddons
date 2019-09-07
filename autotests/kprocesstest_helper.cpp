/*
    This file is part of the KDE libraries

    Copyright (C) 2007 Oswald Buddenhagen <ossi@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kprocess.h>
#include "kprocesstest_helper.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    KProcess p;
    p.setShellCommand(QString::fromLatin1("echo " EOUT "; echo " EERR " >&2"));
    p.setOutputChannelMode(static_cast<KProcess::OutputChannelMode>(atoi(argv[1])));
    fputs(POUT, stdout);
    fflush(stdout);
    p.execute();
    fputs(ROUT, stdout);
    fputs(p.readAllStandardOutput().constData(), stdout);
    fputs(RERR, stdout);
    fputs(p.readAllStandardError().constData(), stdout);
    return 0;
}
