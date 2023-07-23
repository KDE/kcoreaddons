/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kstaticpluginhelpers.h"
#include "kstaticpluginhelpers_p.h"

typedef QHash<QString, QMap<QString, QStaticPlugin>> StaticPluginMap;
Q_GLOBAL_STATIC(StaticPluginMap, s_staticPlugins)

QList<QStaticPlugin> KStaticPluginHelpers::staticPlugins(const QString &directory)
{
    return s_staticPlugins->value(directory).values();
}

std::optional<QStaticPlugin> KStaticPluginHelpers::findById(const QString &directory, const QString &pluginId)
{
    const auto staticPlugins = s_staticPlugins->value(directory);
    const auto it = staticPlugins.constFind(pluginId);
    return it == staticPlugins.end() ? std::nullopt : std::optional(it.value());
}

void kRegisterStaticPluginFunction(const QString &pluginId, const QString &directory, QStaticPlugin plugin)
{
    (*s_staticPlugins)[directory].insert(pluginId, plugin);
}
