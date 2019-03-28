/*
  Copyright (C) 2014-2019 Harald Sitter <sitter@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) version 3, or any
  later version accepted by the membership of KDE e.V. (or its
  successor approved by the membership of KDE e.V.), which shall
  act as a proxy defined in Section 6 of version 3 of the license.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <QTest>

#include "kosrelease.h"

class KOSReleaseTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testParse()
    {
        KOSRelease r(QFINDTESTDATA("data/os-release"));
        QCOMPARE(r.name(), QStringLiteral("Name"));
        QCOMPARE(r.version(), QStringLiteral("100.5"));
        QCOMPARE(r.id(), QStringLiteral("theid"));
        QCOMPARE(r.idLike(), QStringList({ QStringLiteral("otherid"), QStringLiteral("otherotherid") }));
        QCOMPARE(r.versionCodename(), QStringLiteral("versioncodename"));
        QCOMPARE(r.versionId(), QStringLiteral("500.1"));
        QCOMPARE(r.prettyName(), QStringLiteral("Pretty Name #1"));
        QCOMPARE(r.ansiColor(), QStringLiteral("1;34"));
        QCOMPARE(r.cpeName(), QStringLiteral("cpe:/o:foo:bar:100"));
        QCOMPARE(r.homeUrl(), QStringLiteral("https://url.home"));
        QCOMPARE(r.documentationUrl(), QStringLiteral("https://url.docs"));
        QCOMPARE(r.supportUrl(), QStringLiteral("https://url.support"));
        QCOMPARE(r.bugReportUrl(), QStringLiteral("https://url.bugs"));
        QCOMPARE(r.privacyPolicyUrl(), QStringLiteral("https://url.privacy"));
        QCOMPARE(r.buildId(), QStringLiteral("105.5"));
        QCOMPARE(r.variant(), QStringLiteral("Test = Edition"));
        QCOMPARE(r.variantId(), QStringLiteral("test"));
        QCOMPARE(r.logo(), QStringLiteral("start-here-test"));
        QCOMPARE(r.extraKeys(), QStringList({ QStringLiteral("DEBIAN_BTS") }));
        QCOMPARE(r.extraValue(QStringLiteral("DEBIAN_BTS")), QStringLiteral("debbugs://bugs.debian.org/"));
    }
};

QTEST_MAIN(KOSReleaseTest)

#include "kosreleasetest.moc"
