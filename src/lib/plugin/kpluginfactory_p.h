/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINFACTORY_P_H
#define KPLUGINFACTORY_P_H

#include "kpluginfactory.h"
#include <KPluginMetaData>
#include <QMultiHash>

class KPluginFactoryPrivate
{
    friend class KPluginFactory;

public:
    ~KPluginFactoryPrivate() = default;

protected:
    typedef QPair<const QMetaObject *, KPluginFactory::CreateInstanceFunction> Plugin;
    using PluginWithMetadata = QPair<const QMetaObject *, KPluginFactory::CreateInstanceWithMetaDataFunction>;

    KPluginFactoryPrivate() = default;

    KPluginMetaData metaData;
    QMultiHash<QString, Plugin> createInstanceHash;
    QMultiHash<QString, PluginWithMetadata> createInstanceWithMetaDataHash;
};

#endif // KPLUGINFACTORY_P_H
