/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 2000 Charles Samuels <charles@kde.org>
    SPDX-FileCopyrightText: 2005 Joseph Wenninger <kde@jowenn.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "krandom.h"

#include <stdlib.h>
#ifdef Q_OS_WIN
#include <process.h>
#else // Q_OS_WIN
#include <unistd.h>
#endif // Q_OS_WIN
#include <stdio.h>
#include <time.h>
#ifndef Q_OS_WIN
#include <sys/time.h>
#endif //  Q_OS_WIN
#include <fcntl.h>

#include <QFile>
#include <QThread>
#include <QThreadStorage>

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 72)
int KRandom::random()
{
    static QThreadStorage<bool> initialized_threads;
    if (!initialized_threads.localData()) {
        unsigned int seed;
        initialized_threads.setLocalData(true);
        QFile urandom(QStringLiteral("/dev/urandom"));
        bool opened = urandom.open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        if (!opened || urandom.read(reinterpret_cast<char *>(&seed), sizeof(seed)) != sizeof(seed)) {
            // silence warnings about use of deprecated qsrand()/qrand()
            // Porting to QRandomGenerator::global() instead might result in no new seed set for the generator behind qrand()
            // which then might affect other places indirectly relying on this.
            // So just keeping the old calls here, as this method is also deprecated and will disappear together with qsrand/qrand.
            QT_WARNING_PUSH
            QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
            QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
            // No /dev/urandom... try something else.
            qsrand(getpid());
            seed = qrand() ^ time(nullptr) ^ reinterpret_cast<quintptr>(QThread::currentThread());
        }
        qsrand(seed);
    }
    return qrand();
    QT_WARNING_POP
}
#endif

QString KRandom::randomString(int length)
{
    if (length <= 0) {
        return QString();
    }

    QString str;
    str.resize(length);
    int i = 0;
    while (length--) {
        int r = QRandomGenerator::global()->bounded(62);
        r += 48;
        if (r > 57) {
            r += 7;
        }
        if (r > 90) {
            r += 6;
        }
        str[i++] = QLatin1Char(char(r));
        // so what if I work backwards?
    }
    return str;
}
