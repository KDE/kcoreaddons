/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "jsonplugin2.h"
#include <kpluginfactory.h>

JsonPlugin2::JsonPlugin2(QObject *parent, const QVariantList &args)
    : QObject(parent)
{
    Q_UNUSED(args)
}

K_PLUGIN_FACTORY_WITH_JSON(jsonplugin2, "jsonplugin2.json", registerPlugin<JsonPlugin2>();)

#include "jsonplugin2.moc"
