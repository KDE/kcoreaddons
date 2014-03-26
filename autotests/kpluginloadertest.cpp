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
    void initTestCase()
    {
        const QString serviceFile = QFINDTESTDATA("fakeplugin.desktop");
        QVERIFY(!serviceFile.isEmpty());
    }

    void testFindPlugin_missing()
    {
        const QString location = KPluginLoader::findPlugin("idonotexist");
        QVERIFY2(location.isEmpty(), qPrintable(location));
    }

    void testFindPlugin()
    {
        const QString location = KPluginLoader::findPlugin("fakeplugin");
        QVERIFY2(!location.isEmpty(), qPrintable(location));
    }

    void testPluginVersion()
    {
        KPluginLoader vplugin("versionedplugin");
        QCOMPARE(vplugin.pluginVersion(), quint32(5));

        KPluginLoader uplugin("unversionedplugin");
        QCOMPARE(uplugin.pluginVersion(), quint32(-1));

        KService service(QFINDTESTDATA("fakeplugin.desktop"));
        KPluginLoader fplugin(service);
        QCOMPARE(fplugin.pluginVersion(), quint32(-1));

        KPluginLoader noplugin("idonotexist");
        QCOMPARE(noplugin.pluginVersion(), quint32(-1));
    }

    void testPluginName()
    {
        KPluginLoader vplugin("versionedplugin");
        QCOMPARE(vplugin.pluginName(), QString::fromLatin1("versionedplugin"));

        KService service(QFINDTESTDATA("fakeplugin.desktop"));
        KPluginLoader fplugin(service);
        QCOMPARE(fplugin.pluginName(), QString::fromLatin1("fakeplugin"));

        KPluginLoader noplugin("idonotexist");
        QCOMPARE(noplugin.pluginName(), QString::fromLatin1("idonotexist"));
    }

    void testFactory()
    {
        KPluginLoader vplugin("versionedplugin");
        QVERIFY(vplugin.factory());

        KService service(QFINDTESTDATA("fakeplugin.desktop"));
        KPluginLoader fplugin(service);
        QVERIFY(fplugin.factory());

        KPluginLoader noplugin("idonotexist");
        QVERIFY(!noplugin.factory());
    }
};


QTEST_MAIN(KPluginLoaderTest)

#include "kpluginloadertest.moc"
