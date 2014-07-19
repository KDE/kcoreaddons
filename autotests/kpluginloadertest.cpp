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
#include <QFileInfo>

#include <kpluginloader.h>
#include <kpluginmetadata.h>

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
        QCOMPARE(QFileInfo(vplugin.fileName()).canonicalFilePath(),
                 QFileInfo(QStringLiteral(VERSIONEDPLUGIN_FILE)).canonicalFilePath());

        KPluginLoader jplugin(KPluginName("jsonplugin"));
        QCOMPARE(QFileInfo(jplugin.fileName()).canonicalFilePath(),
                 QFileInfo(QStringLiteral(JSONPLUGIN_FILE)).canonicalFilePath());

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
        QJsonValue comment = jmetadata_obj.value("KPlugin").toObject().value("Description");
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

    void testInstantiatePlugins()
    {
        const QString plugin1Path = KPluginLoader::findPlugin("jsonplugin");
        QVERIFY2(!plugin1Path.isEmpty(), qPrintable(plugin1Path));
        const QString plugin2Path = KPluginLoader::findPlugin("unversionedplugin");
        QVERIFY2(!plugin2Path.isEmpty(), qPrintable(plugin2Path));
        const QString plugin3Path = KPluginLoader::findPlugin("jsonplugin2");
        QVERIFY2(!plugin3Path.isEmpty(), qPrintable(plugin3Path));

        QTemporaryDir temp;
        QDir dir(temp.path());
        QVERIFY2(QFile::copy(plugin1Path, dir.absoluteFilePath(QFileInfo(plugin1Path).fileName())),
            qPrintable(dir.absoluteFilePath(QFileInfo(plugin1Path).fileName())));
        QVERIFY2(QFile::copy(plugin2Path, dir.absoluteFilePath(QFileInfo(plugin2Path).fileName())),
            qPrintable(dir.absoluteFilePath(QFileInfo(plugin2Path).fileName())));
        QVERIFY2(QFile::copy(plugin3Path, dir.absoluteFilePath(QFileInfo(plugin3Path).fileName())),
            qPrintable(dir.absoluteFilePath(QFileInfo(plugin3Path).fileName())));

        // only jsonplugin, since unversionedplugin has no json metadata
        QList<QObject*> plugins = KPluginLoader::instantiatePlugins(temp.path());
        QCOMPARE(plugins.size(), 2);
        QStringList classNames = QStringList() << plugins[0]->metaObject()->className()
            << plugins[1]->metaObject()->className();
        classNames.sort();
        QCOMPARE(classNames[0], QStringLiteral("jsonplugin2"));
        QCOMPARE(classNames[1], QStringLiteral("jsonpluginfa"));
        qDeleteAll(plugins);

        //try filter
        plugins = KPluginLoader::instantiatePlugins(temp.path(), [](const KPluginMetaData & md) {
            return md.pluginId() == "jsonplugin";
        });
        QCOMPARE(plugins.size(), 1);
        QCOMPARE(plugins[0]->metaObject()->className(), "jsonpluginfa");
        qDeleteAll(plugins);

        plugins = KPluginLoader::instantiatePlugins(temp.path(), [](const KPluginMetaData & md) {
            return md.pluginId() == "unversionedplugin";
        });
        QCOMPARE(plugins.size(), 0);

        plugins = KPluginLoader::instantiatePlugins(temp.path(), [](const KPluginMetaData & md) {
            return md.pluginId() == "foobar"; // ID does not macht file name, is set in JSON
        });
        QCOMPARE(plugins.size(), 1);
        QCOMPARE(plugins[0]->metaObject()->className(), "jsonplugin2");
        qDeleteAll(plugins);

        // check that parent gets set
        plugins = KPluginLoader::instantiatePlugins(temp.path(), nullptr, this);
        QCOMPARE(plugins.size(), 2);
        QCOMPARE(plugins[0]->parent(), this);
        QCOMPARE(plugins[1]->parent(), this);
        qDeleteAll(plugins);
    }
};

QTEST_MAIN(KPluginLoaderTest)

#include "kpluginloadertest.moc"
