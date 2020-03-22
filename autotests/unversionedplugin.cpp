/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Alex Merry <alexmerry@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "unversionedplugin.h"
#include <kexportplugin.h>
#include <kpluginfactory.h>
#include <QDebug>

UnversionedPlugin::UnversionedPlugin(QObject *parent, const QVariantList &args)
    : QObject(parent)
{
    qDebug() << "Created UnversionedPlugin with args" << args;
}

K_PLUGIN_FACTORY(UnversionedPluginFactory, registerPlugin<UnversionedPlugin>();)

#include "unversionedplugin.moc"
