/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include <QPluginLoader>
#include <kpluginfactory.h>
#include <kpluginloader.h>

// We do not have QWidgets as a dependency, this is a simple placeholder for the type to be fully qualified
class QWidget : public QObject
{
};

class KPluginFactoryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreate()
    {
        KPluginFactory::Result<KPluginFactory> factoryResult = KPluginFactory::loadFactory(KPluginMetaData(QStringLiteral("multiplugin")));
        auto factory = factoryResult.plugin;
        QVERIFY(factory);
        QVariantList args;
        args << QStringLiteral("Some") << QStringLiteral("args") << 5;

        QObject *obj = factory->create<QObject>(this, args);
        QVERIFY(obj);
        QCOMPARE(obj->objectName(), QString::fromLatin1("MultiPlugin1"));

        QObject *obj2 = factory->create<QObject>(this, args);
        QVERIFY(obj2);
        QCOMPARE(obj2->objectName(), QString::fromLatin1("MultiPlugin1"));
        QVERIFY(obj != obj2);
        delete obj;
        delete obj2;

        // Try creating a part without keyword/args
        QWidget parentWidget;
        QObject *partTest = factory->create<QObject>(&parentWidget, this);
        QVERIFY(partTest);
        delete partTest;

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 89)
        obj = factory->create<QObject>(QStringLiteral("secondary"), this, args);
        QVERIFY(obj);
        QCOMPARE(obj->objectName(), QString::fromLatin1("MultiPlugin2"));

        obj2 = factory->create<QObject>(QStringLiteral("secondary"), this, args);
        QVERIFY(obj2);
        QCOMPARE(obj2->objectName(), QString::fromLatin1("MultiPlugin2"));
        QVERIFY(obj != obj2);
        delete obj;
        delete obj2;
#endif
    }

    void testPluginWithoutMetaData()
    {
        KPluginFactory::Result<KPluginFactory> factoryResult = KPluginFactory::loadFactory(KPluginMetaData(QStringLiteral("namespace/pluginwithoutmetadata")));
        QVERIFY(factoryResult);
        auto plugin = factoryResult.plugin->create<QObject>();
        QVERIFY(plugin);
        QCOMPARE(plugin->metaObject()->className(), "PluginWithoutMetaData");
        delete plugin;
    }

    void testResultingCMakeMacroPlugin()
    {
        KPluginMetaData data(QStringLiteral("namespace/jsonplugin_cmake_macro"));
        QVERIFY(data.isValid());

        auto instance = QPluginLoader(data.fileName()).instance();
        QVERIFY(instance);
        QCOMPARE(instance->metaObject()->className(), "jsonplugin_cmake_macro_factory");
    }
    void testCreateUsingUtilityMethods()
    {
        auto result = KPluginFactory::instantiatePlugin<QObject>(KPluginMetaData(QStringLiteral("jsonplugin")), nullptr, QVariantList());
        QVERIFY(result.plugin);
        QCOMPARE(result.plugin->metaObject()->className(), "JsonPlugin");
        QVERIFY(result.errorString.isEmpty());
        QCOMPARE(result.errorReason, KPluginFactory::NO_PLUGIN_ERROR);
        delete result.plugin;
    }

    void testCreateUsingUtilityMethodsErrorHandling()
    {
        {
            auto result = KPluginFactory::instantiatePlugin<QObject>(KPluginMetaData(QFINDTESTDATA("jsonplugin.json")), nullptr, QVariantList());
            QVERIFY(!result.plugin);
            QCOMPARE(result.errorReason, KPluginFactory::INVALID_PLUGIN);
        }
        {
            // it is a valid plugin, but does not contain a KPluginFactory
            QVERIFY(QPluginLoader(QStringLiteral("qtplugin")).instance());
            auto result = KPluginFactory::instantiatePlugin<QObject>(KPluginMetaData(QStringLiteral("qtplugin")), nullptr, QVariantList());
            QVERIFY(!result.plugin);
            // But does not contain a valid plugin factory
            QCOMPARE(result.errorReason, KPluginFactory::INVALID_FACTORY);
        }
        {
            // it is a QObject, but not a KPluginFactoryTest instance
            auto result = KPluginFactory::instantiatePlugin<KPluginFactoryTest>(KPluginMetaData(QStringLiteral("jsonplugin")), nullptr, QVariantList());
            QVERIFY(!result.plugin);
            QCOMPARE(result.errorReason, KPluginFactory::INVALID_KPLUGINFACTORY_INSTANTIATION);
            QVERIFY(result.errorText.contains("KPluginFactoryTest"));
        }
    }

    void testStaticPlugins()
    {
        const auto plugins = KPluginMetaData::findPlugins(QStringLiteral("staticnamespace"));
        QCOMPARE(plugins.count(), 1);

        auto result = KPluginFactory::instantiatePlugin<QObject>(plugins.first());
        QVERIFY(result);
        delete result.plugin;
    }

    void testNonExistingPlugin()
    {
        KPluginMetaData data(QStringLiteral("does/not/exist"));
        QVERIFY(!data.isValid());
        const auto res = KPluginFactory::instantiatePlugin<QObject>(data);
        QVERIFY(!res);
        QCOMPARE(res.errorReason, KPluginFactory::INVALID_PLUGIN);
        QCOMPARE(res.errorText, QStringLiteral("Could not find plugin does/not/exist"));
    }
};

QTEST_MAIN(KPluginFactoryTest)

#include "kpluginfactorytest.moc"
