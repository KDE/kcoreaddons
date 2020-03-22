/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KEXPORTPLUGIN_H
#define KEXPORTPLUGIN_H

#include <QPluginLoader>
#include <QtPlugin>
#include <kcoreaddons_export.h>

/**
 * \relates KPluginLoader
 * Use this macro if you want to give your plugin a version number.
 * You can later access the version number with KPluginLoader::pluginVersion()
 */
#define K_EXPORT_PLUGIN_VERSION(version) \
    Q_EXTERN_C Q_DECL_EXPORT const quint32 kde_plugin_version = version;

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 0)
/**
 * \relates KPluginLoader
 * This macro exports the main object of the plugin. Most times, this will be a KPluginFactory
 * or derived class, but any QObject derived class can be used.
 * Take a look at the documentation of Q_EXPORT_PLUGIN2 for some details.
 */

#if defined (Q_OS_WIN32) && defined(Q_CC_BOR)
#define Q_STANDARD_CALL __stdcall
#else
#define Q_STANDARD_CALL

class KCOREADDONS_DEPRECATED_EXPORT K_EXPORT_PLUGIN_is_deprecated_see_KDE5PORTING
{
};

#define K_EXPORT_PLUGIN(factory) \
    K_EXPORT_PLUGIN_is_deprecated_see_KDE5PORTING dummy;
#endif

#endif

#endif // KEXPORTPLUGIN_H

