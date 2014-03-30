/*
 * Copyright 2014 Alex Merry <alex.merry@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <QtTest>

#include <kpluginloader.h>
#include <kservice.h>

class KPluginLoaderTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testFindPlugin_missing()
    {
        const QString location = KPluginLoader::findPlugin("idonotexist");
        QVERIFY2(location.isEmpty(), qPrintable(location));
    }

    void testFindPlugin()
    {
        const QString location = KPluginLoader::findPlugin("jsonplugin");
        QVERIFY2(!location.isEmpty(), qPrintable(location));
    }

    void testPluginVersion()
    {
        KPluginLoader vplugin("versionedplugin");
        QCOMPARE(vplugin.pluginVersion(), quint32(5));

        KPluginLoader vplugin2("versionedplugin");
        QCOMPARE(vplugin2.pluginVersion(), quint32(5));

        KPluginLoader uplugin("unversionedplugin");
        QCOMPARE(uplugin.pluginVersion(), quint32(-1));

        KPluginLoader jplugin(KPluginName("jsonplugin"));
        QCOMPARE(jplugin.pluginVersion(), quint32(-1));

        KPluginLoader eplugin(KPluginName::fromErrorString("there was an error"));
        QCOMPARE(eplugin.pluginVersion(), quint32(-1));

        KPluginLoader noplugin("idonotexist");
        QCOMPARE(noplugin.pluginVersion(), quint32(-1));
    }

    void testPluginName()
    {
        KPluginLoader vplugin("versionedplugin");
        QCOMPARE(vplugin.pluginName(), QString::fromLatin1("versionedplugin"));

        KPluginLoader jplugin(KPluginName("jsonplugin"));
        QCOMPARE(jplugin.pluginName(), QString::fromLatin1("jsonplugin"));

        KPluginLoader eplugin(KPluginName::fromErrorString("there was an error"));
        QVERIFY2(eplugin.pluginName().isEmpty(), qPrintable(eplugin.pluginName()));

        KPluginLoader noplugin("idonotexist");
        QCOMPARE(noplugin.pluginName(), QString::fromLatin1("idonotexist"));
    }

    void testFactory()
    {
        KPluginLoader vplugin("versionedplugin");
        QVERIFY(vplugin.factory());

        KPluginLoader jplugin(KPluginName("jsonplugin"));
        QVERIFY(jplugin.factory());

        KPluginLoader eplugin(KPluginName::fromErrorString("there was an error"));
        QVERIFY(!eplugin.factory());

        KPluginLoader noplugin("idonotexist");
        QVERIFY(!noplugin.factory());
    }

    void testErrorString()
    {
        KPluginLoader eplugin(KPluginName::fromErrorString("there was an error"));
        QCOMPARE(eplugin.errorString(), QString::fromLatin1("there was an error"));
    }

    void testFileName()
    {
        KPluginLoader vplugin("versionedplugin");
        QCOMPARE(vplugin.fileName(), QString::fromLatin1(VERSIONEDPLUGIN_FILE));

        KPluginLoader jplugin(KPluginName("jsonplugin"));
        QCOMPARE(jplugin.fileName(), QString::fromLatin1(JSONPLUGIN_FILE));

        KPluginLoader eplugin(KPluginName::fromErrorString("there was an error"));
        QVERIFY2(eplugin.fileName().isEmpty(), qPrintable(eplugin.fileName()));

        KPluginLoader noplugin("idonotexist");
        QVERIFY2(noplugin.fileName().isEmpty(), qPrintable(noplugin.fileName()));
    }

    void testInstance()
    {
        KPluginLoader vplugin("versionedplugin");
        QVERIFY(vplugin.instance());

        KPluginLoader jplugin(KPluginName("jsonplugin"));
        QVERIFY(jplugin.instance());

        KPluginLoader eplugin(KPluginName::fromErrorString("there was an error"));
        QVERIFY(!eplugin.instance());

        KPluginLoader noplugin("idonotexist");
        QVERIFY(!noplugin.instance());
    }

    void testIsLoaded()
    {
        KPluginLoader vplugin("versionedplugin");
        QVERIFY(!vplugin.isLoaded());
        QVERIFY(vplugin.load());
        QVERIFY(vplugin.isLoaded());

        KPluginLoader jplugin(KPluginName("jsonplugin"));
        QVERIFY(!jplugin.isLoaded());
        QVERIFY(jplugin.load());
        QVERIFY(jplugin.isLoaded());

        KPluginLoader aplugin("alwaysunloadplugin");
        QVERIFY(!aplugin.isLoaded());
        QVERIFY(aplugin.load());
        QVERIFY(aplugin.isLoaded());
        if (aplugin.unload()) {
            QVERIFY(!aplugin.isLoaded());
        } else {
            qDebug() << "Could not unload alwaysunloadplugin:" << aplugin.errorString();
        }

        KPluginLoader eplugin(KPluginName::fromErrorString("there was an error"));
        QVERIFY(!eplugin.isLoaded());
        QVERIFY(!eplugin.load());
        QVERIFY(!eplugin.isLoaded());

        KPluginLoader noplugin("idonotexist");
        QVERIFY(!noplugin.isLoaded());
        QVERIFY(!noplugin.load());
        QVERIFY(!noplugin.isLoaded());
    }

    void testLoad()
    {
        KPluginLoader vplugin("versionedplugin");
        QVERIFY(vplugin.load());

        KPluginLoader jplugin(KPluginName("jsonplugin"));
        QVERIFY(jplugin.load());

        KPluginLoader eplugin(KPluginName::fromErrorString("there was an error"));
        QVERIFY(!eplugin.load());

        KPluginLoader noplugin("idonotexist");
        QVERIFY(!noplugin.load());
    }

    void testLoadHints()
    {
        KPluginLoader aplugin("alwaysunloadplugin");
        aplugin.setLoadHints(QLibrary::ResolveAllSymbolsHint);
        QCOMPARE(aplugin.loadHints(), QLibrary::ResolveAllSymbolsHint);
    }

    void testMetaData()
    {
        KPluginLoader aplugin("alwaysunloadplugin");
        QJsonObject ametadata = aplugin.metaData();
        QVERIFY(!ametadata.isEmpty());
        QVERIFY(ametadata.keys().contains("IID"));
        QJsonValue ametadata_metadata = ametadata.value("MetaData");
        QVERIFY(ametadata_metadata.toObject().isEmpty());
        QVERIFY(!aplugin.isLoaded()); // didn't load anything

        KPluginLoader jplugin(KPluginName("jsonplugin"));
        QJsonObject jmetadata = jplugin.metaData();
        QVERIFY(!jmetadata.isEmpty());
        QJsonValue jmetadata_metadata = jmetadata.value("MetaData");
        QVERIFY(jmetadata_metadata.isObject());
        QJsonObject jmetadata_obj = jmetadata_metadata.toObject();
        QVERIFY(!jmetadata_obj.isEmpty());
        QJsonValue comment = jmetadata_obj.value("Comment");
        QVERIFY(comment.isString());
        QCOMPARE(comment.toString(), QString::fromLatin1("This is a plugin"));

        KPluginLoader eplugin(KPluginName::fromErrorString("there was an error"));
        QVERIFY(eplugin.metaData().isEmpty());

        KPluginLoader noplugin("idonotexist");
        QVERIFY(noplugin.metaData().isEmpty());
    }

    void testUnload()
    {
        KPluginLoader aplugin("alwaysunloadplugin");
        QVERIFY(aplugin.load());
        // may need QEXPECT_FAIL on some platforms...
        QVERIFY(aplugin.unload());
    }
};


QTEST_MAIN(KPluginLoaderTest)

#include "kpluginloadertest.moc"
