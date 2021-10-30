/*
    SPDX-FileCopyrightText: 2021 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QPluginLoader>

namespace KStaticPluginHelpers
{
/**
 * This is an implementations detail since consumers should only interact with KPluginMetaData::findPlugin*
 * to query the available plugins!
 */
QList<QStaticPlugin> staticPlugins(const QString &directory);
}
