/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include "klibexec.h"
#include <config-util.h>

#if HAVE_DLADDR
#include <dlfcn.h>
#elif defined(Q_OS_WIN)
#include <windows.h>

#include <QVarLengthArray>
#endif

#include <QCoreApplication>
#include <QDir>
#include <QLibraryInfo>

#include <kcoreaddons_debug.h>

static QString libraryPathFromAddress(void *address)
{
#if HAVE_DLADDR
    Dl_info info{};
    if (dladdr(address, &info) == 0) {
        qCWarning(KCOREADDONS_DEBUG) << "Failed to match address to shared object.";
        // Do not call dlerror. It's only expected to return something useful on freebsd!
        return {};
    }
    return QFile::decodeName(info.dli_fname);
#elif defined(Q_OS_WIN)
    HMODULE hModule = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, static_cast<LPWSTR>(address), &hModule)) {
        qCWarning(KCOREADDONS_DEBUG) << "Failed to GetModuleHandleExW" << GetLastError();
        return {};
    }
    if (!hModule) {
        qCWarning(KCOREADDONS_DEBUG) << "hModule null unexpectedly";
        return {};
    }

    QVarLengthArray<wchar_t, MAX_PATH> pathArray;
    DWORD pathSize = pathArray.size();
    while (pathSize == pathArray.size()) { // pathSize doesn't include the null byte on success, so this only ever true if we need to grow
        pathArray.resize(pathArray.size() + MAX_PATH);
        pathSize = GetModuleFileNameW(hModule, pathArray.data(), pathArray.size());
        if (pathSize == 0) {
            qCWarning(KCOREADDONS_DEBUG) << "Failed to GetModuleFileNameW" << GetLastError();
            return {};
        }
    }
    return QDir::fromNativeSeparators(QString::fromWCharArray(pathArray.data()));
#else // unsupported
    return {};
#endif
}

QString KLibexec::pathFromAddress(const QString &relativePath, void *address)
{
    const QString libraryPath = libraryPathFromAddress(address);
    // On usr-merged distros, /lib(64) links to /usr/lib(64) for instance.
    // Libraries are installed into the directories below /usr, but glibc's builtin default means
    // that they get loaded through /lib(64). This breaks some relative paths. canonicalPath()
    // resolves that and should be more reliable.
    const QString canonicalDirPath = QFileInfo(libraryPath).canonicalPath();
    const QString libexecPath = QFileInfo(canonicalDirPath + QLatin1Char('/') + relativePath).absoluteFilePath();
    return libexecPath;
}

QStringList KLibexec::pathCandidates(const QString &relativePath)
{
    const QString qLibexec = QLibraryInfo::location(QLibraryInfo::LibraryExecutablesPath);
    const QString qLibexecKF5 = qLibexec + QLatin1String("/kf5");

    return {
        QCoreApplication::applicationDirPath(), // look where our application binary is located
        qLibexec, // look where libexec path is (can be set in qt.conf)
        qLibexecKF5, // on !win32 we use a kf5 suffix
        relativePath,
    };
}
