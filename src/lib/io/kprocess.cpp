/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2007 Oswald Buddenhagen <ossi@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcoreaddons_debug.h"
#include "kprocess_p.h"

#include <QStandardPaths>
#include <kshell.h>
#include <qplatformdefs.h>
#ifdef Q_OS_WIN
#include <kshell_p.h>
#include <qt_windows.h>
#endif

#include <QFile>

/////////////////////////////
// public member functions //
/////////////////////////////

KProcess::KProcess(QObject *parent)
    : QProcess(parent)
    , d_ptr(new KProcessPrivate(this))
{
    setOutputChannelMode(ForwardedChannels);
}

KProcess::KProcess(KProcessPrivate *d, QObject *parent)
    : QProcess(parent)
    , d_ptr(d)
{
    d_ptr->q_ptr = this;
    setOutputChannelMode(ForwardedChannels);
}

KProcess::~KProcess() = default;

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
    setEnvironment(QStringList{QStringLiteral(DUMMYENV)});
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
    auto it = std::find_if(env.begin(), env.end(), [&fname](const QString &s) {
        return s.startsWith(fname);
    });
    if (it != env.end()) {
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

    auto it = std::find_if(env.begin(), env.end(), [&fname](const QString &s) {
        return s.startsWith(fname);
    });
    if (it != env.end()) {
        env.erase(it);
        if (env.isEmpty()) {
            env.append(QStringLiteral(DUMMYENV));
        }
        setEnvironment(env);
    }
}

void KProcess::setProgram(const QString &exe, const QStringList &args)
{
    Q_D(KProcess);

    QProcess::setProgram(exe);
    QProcess::setArguments(args);
#ifdef Q_OS_WIN
    setNativeArguments(QString());
#endif
}

void KProcess::setProgram(const QStringList &argv)
{
    Q_D(KProcess);

    if (argv.isEmpty()) {
        qCWarning(KCOREADDONS_DEBUG) << "KProcess::setProgram(const QStringList &argv) called on an empty string list, no process will be started.";
        clearProgram();
        return;
    }

    QStringList args = argv;
    QProcess::setProgram(args.takeFirst());
    QProcess::setArguments(args);
#ifdef Q_OS_WIN
    setNativeArguments(QString());
#endif
}

KProcess &KProcess::operator<<(const QString &arg)
{
    Q_D(KProcess);

    if (QProcess::program().isEmpty()) {
        QProcess::setProgram(arg);
    } else {
        setArguments(arguments() << arg);
    }
    return *this;
}

KProcess &KProcess::operator<<(const QStringList &args)
{
    Q_D(KProcess);

    if (QProcess::program().isEmpty()) {
        setProgram(args);
    } else {
        setArguments(arguments() << args);
    }
    return *this;
}

void KProcess::clearProgram()
{
    Q_D(KProcess);

    QProcess::setProgram({});
    QProcess::setArguments({});
#ifdef Q_OS_WIN
    setNativeArguments(QString());
#endif
}

void KProcess::setShellCommand(const QString &cmd)
{
    Q_D(KProcess);

    KShell::Errors err = KShell::NoError;
    auto args = KShell::splitArgs(cmd, KShell::AbortOnMeta | KShell::TildeExpand, &err);
    if (err == KShell::NoError && !args.isEmpty()) {
        QProcess::setProgram(QStandardPaths::findExecutable(args.takeFirst()));
        if (!QProcess::program().isEmpty()) {
            setArguments(args);
#ifdef Q_OS_WIN
            setNativeArguments(QString());
#endif
            return;
        }
    }

    setArguments({});

#ifdef Q_OS_UNIX
// #ifdef NON_FREE // ... as they ship non-POSIX /bin/sh
#if !defined(__linux__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__DragonFly__) && !defined(__GNU__)              \
    && !defined(__APPLE__)
    // If /bin/sh is a symlink, we can be pretty sure that it points to a
    // POSIX shell - the original bourne shell is about the only non-POSIX
    // shell still in use and it is always installed natively as /bin/sh.
    QProcess::setProgram(QFile::symLinkTarget(QStringLiteral("/bin/sh"));
    if (QProcess::program()) {
        // Try some known POSIX shells.
        QProcess::setProgram(QStandardPaths::findExecutable(QStringLiteral("ksh")));
        if (QProcess::program()) {
            QProcess::setProgram(QStandardPaths::findExecutable(QStringLiteral("ash")));
            if (QProcess::program()) {
                QProcess::setProgram(QStandardPaths::findExecutable(QStringLiteral("bash")));
                if (QProcess::program()) {
                    QProcess::setProgram(QStandardPaths::findExecutable(QStringLiteral("zsh")));
                    if (QProcess::program())
                    // We're pretty much screwed, to be honest ...
                    {
                        QProcess::setProgram(QStringLiteral("/bin/sh"));
                    }
                }
            }
        }
    }
#else
    QProcess::setProgram((QStringLiteral("/bin/sh")));
#endif

    setArguments(arguments() << QStringLiteral("-c") << cmd);
#else // Q_OS_UNIX
    // KMacroExpander::expandMacrosShellQuote(), KShell::quoteArg() and
    // KShell::joinArgs() may generate these for security reasons.
    setEnv(PERCENT_VARIABLE, QStringLiteral("%"));

#ifndef _WIN32_WCE
    WCHAR sysdir[MAX_PATH + 1];
    UINT size = GetSystemDirectoryW(sysdir, MAX_PATH + 1);
    QProcess::setProgram(QString::fromUtf16((const ushort *)sysdir, size) + QLatin1String("\\cmd.exe"));
    setNativeArguments(QLatin1String("/V:OFF /S /C \"") + cmd + QLatin1Char('"'));
#else
    QProcess::setProgram(QStringLiteral("\\windows\\cmd.exe"));
    setNativeArguments(QStringLiteral("/S /C \"") + cmd + QLatin1Char('"'));
#endif
#endif
}

QStringList KProcess::program() const
{
    Q_D(const KProcess);

    QStringList argv = arguments();
    argv.prepend(QProcess::program());
    return argv;
}

void KProcess::start()
{
    Q_D(KProcess);

    QProcess::start(d->openMode);
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
    if (!QProcess::startDetached(QProcess::program(), QProcess::arguments(), workingDirectory(), &pid)) {
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
    if (argv.isEmpty()) {
        qCWarning(KCOREADDONS_DEBUG) << "KProcess::startDetached(const QStringList &argv) called on an empty string list, no process will be started.";
        return 0;
    }

    QStringList args = argv;
    QString prog = args.takeFirst();
    return startDetached(prog, args);
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 78)
int KProcess::pid() const
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
    QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
#ifdef Q_OS_UNIX
    return static_cast<int>(QProcess::pid());
#else
    return QProcess::pid() ? QProcess::pid()->dwProcessId : 0;
#endif
    QT_WARNING_POP
}
#endif

#include "moc_kprocess.cpp"
