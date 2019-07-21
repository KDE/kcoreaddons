/* This file is part of the KDE libraries
    Copyright (c) 2000-2005 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kfileutilstest.h"

#include <KFileUtils>

#include <QtTest>

QTEST_MAIN(KFileUtilsTest)

void KFileUtilsTest::testSuggestName_data()
{
    QTest::addColumn<QString>("oldName");
    QTest::addColumn<QStringList>("existingFiles");
    QTest::addColumn<QString>("expectedOutput");

    QTest::newRow("non-existing") << "foobar" << QStringList() << "foobar (1)";
    QTest::newRow("existing") << "foobar" << QStringList(QStringLiteral("foobar")) << "foobar (1)";
    QTest::newRow("existing_1") << "foobar" << (QStringList() << QStringLiteral("foobar") << QStringLiteral("foobar (1)")) << "foobar (2)";
    QTest::newRow("extension") << "foobar.txt" << QStringList() << "foobar (1).txt";
    QTest::newRow("extension_exists") << "foobar.txt" << (QStringList() << QStringLiteral("foobar.txt")) << "foobar (1).txt";
    QTest::newRow("extension_exists_1") << "foobar.txt" << (QStringList() << QStringLiteral("foobar.txt") << QStringLiteral("foobar (1).txt")) << "foobar (2).txt";
    QTest::newRow("two_extensions") << "foobar.tar.gz" << QStringList() << "foobar (1).tar.gz";
    QTest::newRow("two_extensions_exists") << "foobar.tar.gz" << (QStringList() << QStringLiteral("foobar.tar.gz")) << "foobar (1).tar.gz";
    QTest::newRow("two_extensions_exists_1") << "foobar.tar.gz" << (QStringList() << QStringLiteral("foobar.tar.gz") << QStringLiteral("foobar (1).tar.gz")) << "foobar (2).tar.gz";
    QTest::newRow("with_space") << "foo bar" << QStringList(QStringLiteral("foo bar")) << "foo bar (1)";
    QTest::newRow("dot_at_beginning") << ".aFile.tar.gz" << QStringList() << ".aFile (1).tar.gz";
    QTest::newRow("dots_at_beginning") << "..aFile.tar.gz" << QStringList() << "..aFile (1).tar.gz";
    QTest::newRow("empty_basename") << ".txt" << QStringList() << ". (1).txt";
    QTest::newRow("empty_basename_2dots") << "..txt" << QStringList() << ". (1).txt";
    QTest::newRow("basename_with_dots") << "filename.5.3.2.tar.gz" << QStringList() << "filename.5.3.2 (1).tar.gz";
    QTest::newRow("unknown_extension_trashinfo") << "fileFromHome.trashinfo" << QStringList() << "fileFromHome (1).trashinfo";
}

void KFileUtilsTest::testSuggestName()
{
    QFETCH(QString, oldName);
    QFETCH(QStringList, existingFiles);
    QFETCH(QString, expectedOutput);

    QTemporaryDir dir;
    const QUrl baseUrl = QUrl::fromLocalFile(dir.path());
    for (const QString &localFile : qAsConst(existingFiles)) {
        QFile file(dir.path() + QLatin1Char('/') + localFile);
        QVERIFY(file.open(QIODevice::WriteOnly));
    }
    QCOMPARE(KFileUtils::suggestName(baseUrl, oldName), expectedOutput);
}
