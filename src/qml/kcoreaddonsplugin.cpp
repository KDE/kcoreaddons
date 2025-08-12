/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include <KAboutData>
#include <KTextToHTML>

class KTextToHtmlWrapper
{
    Q_GADGET
    Q_INVOKABLE [[nodiscard]] static QString
    convertToHtml(const QString &plainText, const KTextToHTML::Options &options, int maxUrlLen = 4096, int maxAddressLen = 255)
    {
        return KTextToHTML::convertToHtml(plainText, options, maxUrlLen, maxAddressLen);
    }
};

class KCoreAddonsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterSingletonType(uri, 1, 0, "AboutData", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
            return engine->toScriptValue(KAboutData::applicationData());
        });
        qmlRegisterSingletonType(uri, 1, 0, "KTextToHTML", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
            return engine->toScriptValue(KTextToHtmlWrapper());
        });
    }
};

#include "kcoreaddonsplugin.moc"
