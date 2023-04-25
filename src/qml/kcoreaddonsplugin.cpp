/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kcoreaddonsplugin.h"

#include <QtQml>

#include "formats.h"
#include "kuserproxy.h"

#include <KFormat>

static QObject *formats_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new Formats();
}

void KCoreAddonsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QString::fromLatin1(uri) == QLatin1String("org.kde.kcoreaddons"));

    qmlRegisterSingletonType<Formats>(uri, 1, 0, "Format", formats_singletontype_provider);
    qRegisterMetaType<QLocale::FormatType>();

    qmlRegisterUncreatableType<KFormat>(uri, 1, 0, "FormatTypes", QString());

    qmlRegisterType<KUserProxy>(uri, 1, 0, "KUser");
}
