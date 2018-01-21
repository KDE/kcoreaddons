/*
 *  Copyright 2014 David Faure <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

// test object
#include <kdelibs4migration.h>
// Qt
#include <QObject>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

class MigrationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testPaths();
};

void MigrationTest::testPaths()
{
    // Setup
    QTemporaryDir kdehomeDir;
    QVERIFY(kdehomeDir.isValid());
    QString kdehome = kdehomeDir.path();
    qputenv("KDEHOME", QFile::encodeName(kdehome));

    QString oldConfigDir = kdehome + QStringLiteral("/share/config/");
    QVERIFY(QDir().mkpath(oldConfigDir));
    QString oldAppsDir = kdehome + QStringLiteral("/share/apps/");
    QVERIFY(QDir().mkpath(oldAppsDir));
    // Test
    Kdelibs4Migration migration;

    QVERIFY(migration.kdeHomeFound());
    QCOMPARE(migration.saveLocation("config"), oldConfigDir);
    QCOMPARE(migration.saveLocation("data"), oldAppsDir);
}

QTEST_MAIN(MigrationTest)

#include "kdelibs4migrationtest.moc"

