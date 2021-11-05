/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Alex Merry <alexmerry@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "multiplugin.h"
#include "kcoreaddons_debug.h"
#include <QDebug>
#include <kexportplugin.h>
#include <kpluginfactory.h>

MultiPlugin1::MultiPlugin1(QObject *parent, const QVariantList &args)
    : QObject(parent)
{
    qCDebug(KCOREADDONS_DEBUG) << "MultiPlugin1" << args;
    setObjectName(QLatin1String("MultiPlugin1"));
}

MultiPlugin2::MultiPlugin2(QObject *parent, const QVariantList &args)
    : QObject(parent)
{
    qCDebug(KCOREADDONS_DEBUG) << "MultiPlugin2" << args;
    setObjectName(QLatin1String("MultiPlugin2"));
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 89)
K_PLUGIN_FACTORY(MultiPluginFactory,
                 registerPlugin<MultiPlugin1>();
                 registerPlugin<MultiPlugin2>(QLatin1String("secondary")); // keyword
)
#else
K_PLUGIN_FACTORY(MultiPluginFactory, registerPlugin<MultiPlugin1>();)
#endif

#include "multiplugin.moc"
