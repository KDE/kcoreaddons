/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <kpluginfactory.h>

class PluginWithoutMetaData : public QObject
{
    Q_OBJECT
public:
    PluginWithoutMetaData(const QObject *, const QVariantList &)
        : QObject(){

        };
};

K_PLUGIN_CLASS(PluginWithoutMetaData)

#include "pluginwithoutmetadata.moc"
