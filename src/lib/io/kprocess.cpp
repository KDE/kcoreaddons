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

#include "kprocess_p.h"

#include <qstandardpaths.h>
#include <qplatformdefs.h>
#include <kshell.h>
#ifdef Q_OS_WIN
# include <qt_windows.h>
# include <kshell_p.h>
#endif

#include <qfile.h>

/////////////////////////////
// public member functions //
/////////////////////////////

KProcess::KProcess(QObject *parent) :
    QProcess(parent),
    d_ptr(new KProcessPrivate(this))
{
    setOutputChannelMode(ForwardedChannels);
}

KProcess::KProcess(KProcessPrivate *d, QObject *parent) :
    QProcess(parent),
    d_ptr(d)
{
    d_ptr->q_ptr = this;
    setOutputChannelMode(ForwardedChannels);
}

KProcess::~KProcess()
{
    delete d_ptr;
}

void KProcess::setOutputChannelMode(OutputChannelMode mode)
{
    QProcess::setProcessChannelMode(static_cast<ProcessChannelMode>(mode));
}

KProcess::OutputChannelMode KProcess::outputChannelMode() const
{
    return static_cast<OutputChannelMode>(QProcess::processChannelMode());
}

void KProcess::setNextOpenMode(QIODevice::OpenMode mode)
{
    Q_D(KProcess);

    d->openMode = mode;
}

#define DUMMYENV "_KPROCESS_DUMMY_="

void KProcess::clearEnvironment()
{
    setEnvironment(QStringList() << QStringLiteral(DUMMYENV));
}

void KProcess::setEnv(const QString &name, const QString &value, bool overwrite)
{
    QStringList env = environment();
    if (env.isEmpty()) {
        env = systemEnvironment();
        env.removeAll(QStringLiteral(DUMMYENV));
    }
    QString fname(name);
    fname.append(QLatin1Char('='));
    for (QStringList::Iterator it = env.begin(); it != env.end(); ++it)
        if ((*it).startsWith(fname)) {
            if (overwrite) {
                *it = fname.append(value);
                setEnvironment(env);
            }
            return;
        }
    env.append(fname.append(value));
    setEnvironment(env);
}

void KProcess::unsetEnv(const QString &name)
{
    QStringList env = environment();
    if (env.isEmpty()) {
        env = systemEnvironment();
        env.removeAll(QStringLiteral(DUMMYENV));
    }
    QString fname(name);
    fname.append(QLatin1Char('='));
    for (QStringList::Iterator it = env.begin(); it != env.end(); ++it)
        if ((*it).startsWith(fname)) {
            env.erase(it);
            if (env.isEmpty()) {
                env.append(QStringLiteral(DUMMYENV));
            }
            setEnvironment(env);
            return;
        }
}

void KProcess::setProgram(const QString &exe, const QStringList &args)
{
    Q_D(KProcess);

    d->prog = exe;
    d->args = args;
#ifdef Q_OS_WIN
    setNativeArguments(QString());
#endif
}

void KProcess::setProgram(const QStringList &argv)
{
    Q_D(KProcess);

    Q_ASSERT(!argv.isEmpty());
    d->args = argv;
    d->prog = d->args.takeFirst();
#ifdef Q_OS_WIN
    setNativeArguments(QString());
#endif
}

KProcess &KProcess::operator<<(const QString &arg)
{
    Q_D(KProcess);

    if (d->prog.isEmpty()) {
        d->prog = arg;
    } else {
        d->args << arg;
    }
    return *this;
}

KProcess &KProcess::operator<<(const QStringList &args)
{
    Q_D(KProcess);

    if (d->prog.isEmpty()) {
        setProgram(args);
    } else {
        d->args << args;
    }
    return *this;
}

void KProcess::clearProgram()
{
    Q_D(KProcess);

    d->prog.clear();
    d->args.clear();
#ifdef Q_OS_WIN
    setNativeArguments(QString());
#endif
}

void KProcess::setShellCommand(const QString &cmd)
{
    Q_D(KProcess);

    KShell::Errors err;
    d->args = KShell::splitArgs(
                  cmd, KShell::AbortOnMeta | KShell::TildeExpand, &err);
    if (err == KShell::NoError && !d->args.isEmpty()) {
        d->prog = QStandardPaths::findExecutable(d->args[0]);
        if (!d->prog.isEmpty()) {
            d->args.removeFirst();
#ifdef Q_OS_WIN
            setNativeArguments(QString());
#endif
            return;
        }
    }

    d->args.clear();

#ifdef Q_OS_UNIX
// #ifdef NON_FREE // ... as they ship non-POSIX /bin/sh
# if !defined(__linux__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__) && !defined(__GNU__) && !defined(__APPLE__)
    // If /bin/sh is a symlink, we can be pretty sure that it points to a
    // POSIX shell - the original bourne shell is about the only non-POSIX
    // shell still in use and it is always installed natively as /bin/sh.
    d->prog = QFile::symLinkTarget(QStringLiteral("/bin/sh"));
    if (d->prog.isEmpty()) {
        // Try some known POSIX shells.
        d->prog = QStandardPaths::findExecutable(QStringLiteral("ksh"));
        if (d->prog.isEmpty()) {
            d->prog = QStandardPaths::findExecutable(QStringLiteral("ash"));
            if (d->prog.isEmpty()) {
                d->prog = QStandardPaths::findExecutable(QStringLiteral("bash"));
                if (d->prog.isEmpty()) {
                    d->prog = QStandardPaths::findExecutable(QStringLiteral("zsh"));
                    if (d->prog.isEmpty())
                        // We're pretty much screwed, to be honest ...
                    {
                        d->prog = QStringLiteral("/bin/sh");
                    }
                }
            }
        }
    }
# else
    d->prog = QStringLiteral("/bin/sh");
# endif

    d->args << QStringLiteral("-c") << cmd;
#else // Q_OS_UNIX
    // KMacroExpander::expandMacrosShellQuote(), KShell::quoteArg() and
    // KShell::joinArgs() may generate these for security reasons.
    setEnv(PERCENT_VARIABLE, QStringLiteral("%"));

#ifndef _WIN32_WCE
    WCHAR sysdir[MAX_PATH + 1];
    UINT size = GetSystemDirectoryW(sysdir, MAX_PATH + 1);
    d->prog = QString::fromUtf16((const ushort *) sysdir, size);
    d->prog += QLatin1String("\\cmd.exe");
    setNativeArguments(QStringLiteral("/V:OFF /S /C \"") + cmd + QLatin1Char('"'));
#else
    d->prog = QStringLiteral("\\windows\\cmd.exe");
    setNativeArguments(QStringLiteral("/S /C \"") + cmd + QLatin1Char('"'));
#endif
#endif
}

QStringList KProcess::program() const
{
    Q_D(const KProcess);

    QStringList argv = d->args;
    argv.prepend(d->prog);
    return argv;
}

void KProcess::start()
{
    Q_D(KProcess);

    QProcess::start(d->prog, d->args, d->openMode);
}

int KProcess::execute(int msecs)
{
    start();
    if (!waitForFinished(msecs)) {
        kill();
        waitForFinished(-1);
        return -2;
    }
    return (exitStatus() == QProcess::NormalExit) ? exitCode() : -1;
}

// static
int KProcess::execute(const QString &exe, const QStringList &args, int msecs)
{
    KProcess p;
    p.setProgram(exe, args);
    return p.execute(msecs);
}

// static
int KProcess::execute(const QStringList &argv, int msecs)
{
    KProcess p;
    p.setProgram(argv);
    return p.execute(msecs);
}

int KProcess::startDetached()
{
    Q_D(KProcess);

    qint64 pid;
    if (!QProcess::startDetached(d->prog, d->args, workingDirectory(), &pid)) {
        return 0;
    }
    return static_cast<int>(pid);
}

// static
int KProcess::startDetached(const QString &exe, const QStringList &args)
{
    qint64 pid;
    if (!QProcess::startDetached(exe, args, QString(), &pid)) {
        return 0;
    }
    return static_cast<int>(pid);
}

// static
int KProcess::startDetached(const QStringList &argv)
{
    QStringList args = argv;
    QString prog = args.takeFirst();
    return startDetached(prog, args);
}

int KProcess::pid() const
{
#ifdef Q_OS_UNIX
    return static_cast<int>(QProcess::pid());
#else
    return QProcess::pid() ? QProcess::pid()->dwProcessId : 0;
#endif
}

#include "moc_kprocess.cpp"
