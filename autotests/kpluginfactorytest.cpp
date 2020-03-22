/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include <kpluginloader.h>
#include <kpluginfactory.h>

class KPluginFactoryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreate()
    {
        KPluginLoader multiplugin(QStringLiteral("multiplugin"));
        KPluginFactory *factory = multiplugin.factory();
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
};

QTEST_MAIN(KPluginFactoryTest)

#include "kpluginfactorytest.moc"

