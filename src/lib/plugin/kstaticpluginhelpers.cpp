/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kstaticpluginhelpers.h"
#include "kstaticpluginhelpers_p.h"

typedef QMultiHash<QString, QStaticPlugin> StaticPluginMap;
Q_GLOBAL_STATIC(StaticPluginMap, s_staticPlugins)

QList<QStaticPlugin> KStaticPluginHelpers::staticPlugins(const QString &directory)
{
    return s_staticPlugins->values(directory);
}

void kRegisterStaticPluginFunction(const QString &directory, QStaticPlugin plugin)
{
    s_staticPlugins->insert(directory, plugin);
}
