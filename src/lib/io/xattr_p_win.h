/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Raphael Kubo da Costa <rakuco@FreeBSD.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FILESYSTEMMETADATA_XATTR_P_H
#define FILESYSTEMMETADATA_XATTR_P_H

#include <QDebug>
#include <QFile>
#include <dirent.h>

#include <QFileInfo>
#include <windows.h>
#define ssize_t SSIZE_T

namespace
{
inline ssize_t k_getxattr(const QString &path, QStringView name, QString *value)
{
    const QString fullADSName = path + QLatin1String(":user.") + name;
    using unique_file_t = std::unique_ptr<HANDLE, decltype(&CloseHandle)>;
    unique_file_t hFile(::CreateFileW(reinterpret_cast<const WCHAR *>(fullADSName.utf16()),
                                      GENERIC_READ,
                                      FILE_SHARE_READ,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_FLAG_SEQUENTIAL_SCAN,
                                      NULL),
                        &CloseHandle);

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = ::GetLastError();
        std::string message = std::system_category().message(error);
        qCWarning(KCOREADDONS_DEBUG) << "failed to open ADS:" << message << fullADSName;
        return -1;
    }

    LARGE_INTEGER lsize;
    BOOL ret = GetFileSizeEx(hFile, &lsize);

    if (!ret || lsize.QuadPart > 0x7fffffff || lsize.QuadPart == 0) {
        value->clear();
        return lsize.QuadPart == 0 ? 0 : -1;
    }

    DWORD r = 0;
    QByteArray data(lsize.QuadPart, Qt::Uninitialized);
    // should we care about attributes longer than 2GiB? - unix xattr are restricted to much lower values
    ret = ::ReadFile(hFile, data.data(), data.size(), &r, NULL);

    if (!ret) {
        DWORD error = ::GetLastError();
        std::string message = std::system_category().message(error);
        qCWarning(KCOREADDONS_DEBUG) << "failed to open ADS:" << message << fullADSName;
        return -1;
    }

    if (r == 0) {
        value->clear();
        return 0;
    }

    data.resize(r);

    *value = QString::fromUtf8(data);
    return r;
}

inline int k_setxattr(const QString &path, const QString &name, const QString &value)
{
    const QString fullADSName = path + QLatin1String(":user.") + name;
    if (fullADSName.size() > MAX_PATH) {
        // https://learn.microsoft.com/en-us/windows/win32/fileio/naming-a-file#maximum-path-length-limitation
        // We could handle longer file names but it would require special casing per-windows version
        return ERROR_FILENAME_EXCED_RANGE;
    }

    HANDLE hFile = ::CreateFileW(reinterpret_cast<const WCHAR *>(fullADSName.utf16()),
                                 GENERIC_WRITE,
                                 0,
                                 NULL,
                                 CREATE_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                 NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = ::GetLastError();
        std::string message = std::system_category().message(error);
        qCWarning(KCOREADDONS_DEBUG) << "failed to open file to write to ADS:" << message << error << fullADSName;
        return -1; // unknown error
    }

    DWORD count = 0;

    const QByteArray v = value.toUtf8();
    if (!::WriteFile(hFile, v.constData(), v.size(), &count, NULL)) {
        DWORD error = ::GetLastError();
        std::string message = std::system_category().message(error);
        qCWarning(KCOREADDONS_DEBUG) << "failed to write to ADS:" << message << error << fullADSName;

        CloseHandle(hFile);
        return -1; // unknown error
    }

    CloseHandle(hFile);
    return 0; // Success
}

inline bool k_hasAttribute(const QString &path, QStringView name)
{
    QString fullpath = path + QLatin1String(":user.") + name;
    HANDLE hFile = CreateFileW(reinterpret_cast<const WCHAR *>(fullpath.utf16()),
                               GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        // ERROR_FILE_NOT_FOUND supposedly
        return false;
    }
    // ADS exists
    CloseHandle(hFile);
    return true;
}

inline int k_removexattr(const QString &path, QStringView name)
{
    const QString fullADSName = path + QLatin1String(":user.") + name;
    int ret = (DeleteFileW(reinterpret_cast<const WCHAR *>(fullADSName.utf16()))) ? 0 : -1;
    return ret;
}

inline bool k_isSupported(const QString &path)
{
    QFileInfo f(path);
    const QString drive = QString(f.absolutePath().left(2)) + QStringLiteral("\\");
    WCHAR szFSName[MAX_PATH];
    DWORD dwVolFlags;
    ::GetVolumeInformationW(reinterpret_cast<const WCHAR *>(drive.utf16()), NULL, 0, NULL, NULL, &dwVolFlags, szFSName, MAX_PATH);
    return ((dwVolFlags & FILE_NAMED_STREAMS) && _wcsicmp(szFSName, L"NTFS") == 0);
}

QStringList k_queryAttributes(QStringView path)
{
    QStringList fileAttributes;
    if (!k_isSupported(path)) {
        return fileAttributes;
    }

    HANDLE hFile = ::CreateFile(reinterpret_cast<const WCHAR *>(path.utf16()),
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD error = ::GetLastError();
        std::string message = std::system_category().message(error);
        qCWarning(KCOREADDONS_DEBUG) << "failed CreateFile:" << message << path << error;
        return fileAttributes;
    }

    QList<QByteArray> entries;
    QString entry;
    FILE_STREAM_INFO *fi = new FILE_STREAM_INFO[256];
    if (::GetFileInformationByHandleEx(hFile, FileStreamInfo, fi, 256 * sizeof(FILE_STREAM_INFO))) {
        // ignore first entry it is "::$DATA"
        FILE_STREAM_INFO *p = fi;
        while (p->NextEntryOffset != NULL) {
            p = (FILE_STREAM_INFO *)((char *)p + p->NextEntryOffset);
            entry = QString::fromUtf16((char16_t *)p->StreamName, p->StreamNameLength / sizeof(char16_t));
            // entries are of the form ":user.key:$DATA"
            entry.chop(6);
            entries.append(entry.sliced(1).toLocal8Bit());
        }
    } else {
        DWORD error = ::GetLastError();
        if (error != ERROR_HANDLE_EOF) {
            std::string message = std::system_category().message(error);
            qCWarning(KCOREADDONS_DEBUG) << "failed GetFileInformationByHandleEx:" << message << path << hFile;
        }
    }
    delete[] fi;
    CloseHandle(hFile);

    if (entries.size() == 0) {
        return fileAttributes;
    }

    const QByteArrayView prefix("user.");
    for (const auto &entry : entries) {
        if (!entry.startsWith(prefix)) {
            continue;
        }
        fileAttributes.append(QString::fromLocal8Bit(entry).sliced(prefix.size()));
    }
    return fileAttributes;
}
}

#endif // FILESYSTEMMETADATA_XATTR_P_H
