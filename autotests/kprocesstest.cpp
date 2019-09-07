/*
    This file is part of the KDE libraries

    Copyright (C) 2007 Oswald Buddenhagen <ossi@kde.org>

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

#include "kprocesstest_helper.h"
#include <kprocess.h>
#include <QObject>
#include <QFile>
#include <QTest>
#include <QStandardPaths>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

class KProcessTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void test_channels();
    void test_setShellCommand();
};

// IOCCC nomination pending

static QString callHelper(KProcess::OutputChannelMode how)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);

    QString helper = QCoreApplication::applicationDirPath() + QStringLiteral("/kprocesstest_helper");
#ifdef Q_OS_WIN
    helper += QStringLiteral(".exe");
#endif

    Q_ASSERT(QFile::exists(helper));
    p.start(helper, QStringList() << QString::number(how) << QStringLiteral("--nocrashhandler"));
    p.waitForFinished();
    return QString::fromLatin1(p.readAllStandardOutput());
}

#define EO EOUT "\n"
#define EE EERR "\n"
#define TESTCHAN(me,ms,pout,rout,rerr) \
    e = QStringLiteral("mode: " ms "\n" POUT pout ROUT rout RERR rerr); \
    a = QStringLiteral("mode: " ms "\n") + callHelper(KProcess::me); \
    QCOMPARE(a, e)

void KProcessTest::test_channels()
{
#ifdef Q_OS_UNIX
    QString e, a;
    TESTCHAN(SeparateChannels, "separate", "", EO, EE);
    TESTCHAN(ForwardedChannels, "forwarded", EO EE, "", "");
    TESTCHAN(OnlyStderrChannel, "forwarded stdout", EO, "", EE);
    TESTCHAN(OnlyStdoutChannel, "forwarded stderr", EE, EO, "");
    TESTCHAN(MergedChannels, "merged", "", EO EE, "");
#else
    Q_UNUSED(recurse);
    QSKIP("This test needs a UNIX system");
#endif
}

void KProcessTest::test_setShellCommand()
{
    // Condition copied from kprocess.cpp
#if !defined(__linux__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__) && !defined(__GNU__)
    QSKIP("This test needs a free UNIX system");
#else
    KProcess p;

    p.setShellCommand(QStringLiteral("cat"));
    QCOMPARE(p.program().count(), 1);
    QCOMPARE(p.program().at(0), QStandardPaths::findExecutable(QStringLiteral("cat")));
    QVERIFY(p.program().at(0).endsWith(QLatin1String("/bin/cat")));
    p.setShellCommand(QStringLiteral("true || false"));
    QCOMPARE(p.program(), QStringList() << QStringLiteral("/bin/sh") << QStringLiteral("-c")
             << QString::fromLatin1("true || false"));
#endif
}

QTEST_MAIN(KProcessTest)

#include "kprocesstest.moc"
