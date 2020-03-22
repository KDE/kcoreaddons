/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "jsonplugin.h"
#include <kpluginfactory.h>

JsonPlugin::JsonPlugin(QObject *parent, const QVariantList &args)
    : QObject(parent)
{
    Q_UNUSED(args)
}

K_PLUGIN_FACTORY_WITH_JSON(jsonpluginfa, "jsonplugin.json", registerPlugin<JsonPlugin>();)

#include "jsonplugin.moc"
