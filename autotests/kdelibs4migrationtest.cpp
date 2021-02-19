/*
    SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

// test object
#include <kdelibs4migration.h>
// Qt
#include <QFile>
#include <QObject>
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
