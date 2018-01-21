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

