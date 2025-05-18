/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Raphael Kubo da Costa <rakuco@FreeBSD.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FILESYSTEMMETADATA_XATTR_P_H
#define FILESYSTEMMETADATA_XATTR_P_H

namespace
{
inline ssize_t k_getxattr(const QString &, const QString &, QString *)
{
    return 0;
}

inline int k_setxattr(const QString &, const QString &, const QString &)
{
    return -1;
}

inline int k_removexattr(const QString &, const QString &)
{
    return -1;
}

inline bool k_hasAttribute(const QString &, const QString &)
{
    return false;
}

inline bool k_isSupported(const QString &)
{
    return false;
}

}

#endif // FILESYSTEMMETADATA_XATTR_P_H
