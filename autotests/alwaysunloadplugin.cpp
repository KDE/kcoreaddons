/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Alex Merry <alexmerry@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "alwaysunloadplugin.h"
#include <kexportplugin.h>
#include <kpluginfactory.h>
#include <QDebug>

AlwaysUnloadPlugin::AlwaysUnloadPlugin(QObject *parent, const QVariantList &args)
    : QObject(parent)
{
    qDebug() << "Created AlwaysUnloadPlugin with args" << args;
}

K_PLUGIN_FACTORY(AlwaysUnloadPluginFactory, registerPlugin<AlwaysUnloadPlugin>();)

#include "alwaysunloadplugin.moc"
