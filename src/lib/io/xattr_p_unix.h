/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Raphael Kubo da Costa <rakuco@FreeBSD.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FILESYSTEMMETADATA_XATTR_P_H
#define FILESYSTEMMETADATA_XATTR_P_H

#include <QDebug>
#include <QFile>

#if defined(Q_OS_LINUX) || defined(__GLIBC__)
#include <sys/types.h>
#include <sys/xattr.h>

#if defined(Q_OS_ANDROID) || defined(Q_OS_LINUX)
// attr/xattr.h is not available in the Android NDK so we are defining ENOATTR ourself
#ifndef ENOATTR
#define ENOATTR ENODATA /* No such attribute */
#endif
#endif

#include <errno.h>
#elif defined(Q_OS_MAC)
#include <errno.h>
#include <sys/types.h>
#include <sys/xattr.h>
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include <errno.h>
#include <sys/extattr.h>
#include <sys/types.h>
#elif defined(Q_OS_OPENBSD)
#include <errno.h>
#endif

namespace
{
inline ssize_t k_getxattr(const QString &path, QStringView name, QString *value)
{
    const QByteArray p = QFile::encodeName(path);
    const char *encodedPath = p.constData();

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    const QByteArray n = name.toUtf8();
#else
    const QByteArray n = QByteArrayView("user.") + name.toUtf8();
#endif
    const char *attributeName = n.constData();

    // First get the size of the data we are going to get to reserve the right amount of space.
#if defined(Q_OS_LINUX) || (defined(__GLIBC__) && !defined(__stub_getxattr))
    const ssize_t size = getxattr(encodedPath, attributeName, nullptr, 0);
#elif defined(Q_OS_MAC)
    const ssize_t size = getxattr(encodedPath, attributeName, NULL, 0, 0, 0);
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    const ssize_t size = extattr_get_file(encodedPath, EXTATTR_NAMESPACE_USER, attributeName, NULL, 0);
#endif

    if (!value) {
        return size;
    }

    if (size <= 0) {
        value->clear();
        return size;
    }

    QByteArray data(size, Qt::Uninitialized);

    while (true) {
#if defined(Q_OS_LINUX) || (defined(__GLIBC__) && !defined(__stub_getxattr))
        const ssize_t r = getxattr(encodedPath, attributeName, data.data(), data.size());
#elif defined(Q_OS_MAC)
        const ssize_t r = getxattr(encodedPath, attributeName, data.data(), data.size(), 0, 0);
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
        const ssize_t r = extattr_get_file(encodedPath, EXTATTR_NAMESPACE_USER, attributeName, data.data(), data.size());
#endif

        if (r < 0 && errno != ERANGE) {
            value->clear();
            return r;
        }

        if (r >= 0) {
            data.resize(r);
            *value = QString::fromUtf8(data);
            return size;
        } else {
            // ERANGE
            data.resize(data.size() * 2);
        }
    }
}

inline int k_setxattr(const QString &path, const QString &name, const QString &value)
{
    const QByteArray p = QFile::encodeName(path);
    const char *encodedPath = p.constData();

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    const QByteArray n = name.toUtf8();
#else
    const QByteArray n = QByteArrayView("user.") + name.toUtf8();
#endif
    const char *attributeName = n.constData();

    const QByteArray v = value.toUtf8();
    const void *attributeValue = v.constData();

    const size_t valueSize = v.size();

#if defined(Q_OS_LINUX) || (defined(__GLIBC__) && !defined(__stub_setxattr))
    int result = setxattr(encodedPath, attributeName, attributeValue, valueSize, 0);
    return result == -1 ? errno : 0;
#elif defined(Q_OS_MAC)
    int count = setxattr(encodedPath, attributeName, attributeValue, valueSize, 0, 0);
    return count == -1 ? errno : 0;
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    const ssize_t count = extattr_set_file(encodedPath, EXTATTR_NAMESPACE_USER, attributeName, attributeValue, valueSize);
    return count == -1 ? errno : 0;
#endif
}

inline int k_removexattr(const QString &path, QStringView name)
{
    const QByteArray p = QFile::encodeName(path);
    const char *encodedPath = p.constData();

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    const QByteArray n = name.toUtf8();
#else
    const QByteArray n = QByteArrayView("user.") + name.toUtf8();
#endif
    const char *attributeName = n.constData();

#if defined(Q_OS_LINUX) || (defined(__GLIBC__) && !defined(__stub_removexattr))
    int result = removexattr(encodedPath, attributeName);
    return result == -1 ? errno : 0;
#elif defined(Q_OS_MAC)
    int result = removexattr(encodedPath, attributeName, XATTR_NOFOLLOW);
    return result == -1 ? errno : 0;
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    int result = extattr_delete_file(encodedPath, EXTATTR_NAMESPACE_USER, attributeName);
    return result == -1 ? errno : 0;
#endif
}

inline bool k_hasAttribute(const QString &path, QStringView name)
{
    auto ret = k_getxattr(path, name, nullptr);
    return (ret >= 0);
}

inline bool k_isSupported(const QString &path)
{
    auto ret = k_getxattr(path, QStringLiteral("test"), nullptr);
    return (ret >= 0) || (errno != ENOTSUP);
}

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
static QList<QByteArray> _split_length_value(QByteArray data)
{
    int pos = 0;
    QList<QByteArray> entries;

    while (pos < data.size()) {
        unsigned char len = data[pos];
        if (pos + 1 + len <= data.size()) {
            auto value = data.mid(pos + 1, len);
            entries.append(value);
        }
        pos += 1 + len;
    }
    return entries;
}
#endif

QStringList k_queryAttributes(QStringView path)
{
    const QByteArray p = path.toLocal8Bit();
    const char *encodedPath = p.constData();
    QStringList fileAttributes;

#if defined(Q_OS_LINUX)
    const ssize_t size = listxattr(encodedPath, nullptr, 0);
#elif defined(Q_OS_MAC)
    const ssize_t size = listxattr(encodedPath, nullptr, 0, 0);
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    const ssize_t size = extattr_list_file(encodedPath, EXTATTR_NAMESPACE_USER, nullptr, 0);
#endif

    if (size == 0) {
        return fileAttributes;
    }

    if (size < 0) {
        return fileAttributes;
    }

    QByteArray data(size, Qt::Uninitialized);

    while (true) {
#if defined(Q_OS_LINUX)
        const ssize_t r = listxattr(encodedPath, data.data(), data.size());
#elif defined(Q_OS_MAC)
        const ssize_t r = listxattr(encodedPath, data.data(), data.size(), 0);
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
        const ssize_t r = extattr_list_file(encodedPath, EXTATTR_NAMESPACE_USER, data.data(), data.size());
#endif

        if (r > 0) {
            data.resize(r);
            break;
        } else {
            data.resize(data.size() * 2);
        }
    }

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    const QByteArrayView prefix;
    const auto entries = _split_length_value(data);
#else
    const QByteArrayView prefix("user.");
    const auto entries = data.split('\0');
#endif

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
