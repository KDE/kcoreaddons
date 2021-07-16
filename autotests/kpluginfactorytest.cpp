/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include <kpluginfactory.h>
#include <kpluginloader.h>

class KPluginFactoryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreate()
    {
        KPluginFactory *factory = KPluginFactory::loadFactory(KPluginMetaData(QStringLiteral("multiplugin")));
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

        obj = factory->create<QObject>(QStringLiteral("secondary"), this, args);
        QVERIFY(obj);
        QCOMPARE(obj->objectName(), QString::fromLatin1("MultiPlugin2"));

        obj2 = factory->create<QObject>(QStringLiteral("secondary"), this, args);
        QVERIFY(obj2);
        QCOMPARE(obj2->objectName(), QString::fromLatin1("MultiPlugin2"));
        QVERIFY(obj != obj2);
        delete obj;
        delete obj2;
    }

    void testCreateUsingUtilityMethods()
    {
        KPluginFactory::KPluginLoadingError error;
        auto *plugin = KPluginFactory::instantiatePlugin<QObject>(KPluginMetaData(QStringLiteral("jsonplugin")), nullptr, QVariantList(), &error);
        QVERIFY(plugin);
        QCOMPARE(plugin->metaObject()->className(), "JsonPlugin");
        QVERIFY(error.errorString.isEmpty());
        QCOMPARE(error.reason, KPluginFactory::KPluginLoadingError::NO_ERROR);
    }

    void testCreateUsingUtilityMethodsErrorHandling()
    {
        {
            KPluginFactory::KPluginLoadingError error;
            auto *plugin = KPluginFactory::instantiatePlugin<QObject>(KPluginMetaData(QFINDTESTDATA("jsonplugin.json")), nullptr, QVariantList(), &error);
            QVERIFY(!plugin);
            QCOMPARE(error.reason, KPluginFactory::KPluginLoadingError::INVALID_PLUGIN);
        }
        {
            KPluginFactory::KPluginLoadingError error;
            // it is a valid plugin, but does not contain a KPluginFactory
            QVERIFY(QPluginLoader(QStringLiteral("qtplugin")).instance());
            auto *plugin = KPluginFactory::instantiatePlugin<QObject>(KPluginMetaData(QStringLiteral("qtplugin")), nullptr, QVariantList(), &error);
            QVERIFY(!plugin);
            // But does not contain a valid plugin factory
            QCOMPARE(error.reason, KPluginFactory::KPluginLoadingError::INVALID_FACTORY);
        }
        {
            KPluginFactory::KPluginLoadingError error;
            // it is a QObject, but not a KPluginFactoryTest instance
            auto *plugin =
                KPluginFactory::instantiatePlugin<KPluginFactoryTest>(KPluginMetaData(QStringLiteral("jsonplugin")), nullptr, QVariantList(), &error);
            QVERIFY(!plugin);
            QCOMPARE(error.reason, KPluginFactory::KPluginLoadingError::INVALID_KPLUGINFACTORY_INSTANTIATION);
        }
    }
};

QTEST_MAIN(KPluginFactoryTest)

#include "kpluginfactorytest.moc"
