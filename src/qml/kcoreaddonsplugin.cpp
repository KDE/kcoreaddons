/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QQmlExtensionPlugin>
#include <QtQml>

#include "formats.h"
#include "kuserproxy.h"
#include <KFormat>

static QObject *formats_singletontype_provider(QQmlEngine *, QJSEngine *)
{
    return new Formats();
}

class KCoreAddonsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterSingletonType<Formats>(uri, 1, 0, "Format", formats_singletontype_provider);
        qRegisterMetaType<QLocale::FormatType>();

        qmlRegisterUncreatableType<KFormat>(uri, 1, 0, "FormatTypes", QString());

        qmlRegisterType<KUserProxy>(uri, 1, 0, "KUser");
    }
};

#include "kcoreaddonsplugin.moc"
