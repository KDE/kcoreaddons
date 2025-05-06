/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ksystemclockskewnotifierengine_p.h"

#include "config-util.h"

#if defined(Q_OS_LINUX)
#include "ksystemclockskewnotifierengine_linux.h"
#elif HAVE_QTDBUS
#include "ksystemclockskewnotifierengine_dbus.h"
#endif

KSystemClockSkewNotifierEngine *KSystemClockSkewNotifierEngine::create(QObject *parent)
{
#if defined(Q_OS_LINUX)
    return KLinuxSystemClockSkewNotifierEngine::create(parent);
#elif HAVE_QTDBUS
    return KDBusSystemClockSkewNotifierEngine::create(parent);
#else
    return nullptr;
#endif
}

KSystemClockSkewNotifierEngine::KSystemClockSkewNotifierEngine(QObject *parent)
    : QObject(parent)
{
}

#include "moc_ksystemclockskewnotifierengine_p.cpp"
