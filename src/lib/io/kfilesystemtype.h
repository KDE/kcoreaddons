/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2011 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only
*/

#ifndef KFILESYSTEMTYPE_P_H
#define KFILESYSTEMTYPE_P_H

#include <QString>
#include <kcoreaddons_export.h>

/**
 * @namespace KFileSystemType
 * Provides utility functions for the type of file systems.
 */
namespace KFileSystemType
{
enum Type {
    Unknown,
    Nfs, ///< NFS or other full-featured networked filesystems (autofs, subfs, cachefs, sshfs)
    Smb, ///< SMB/CIFS mount (networked but with some FAT-like behavior)
    Fat,  ///< FAT or similar (msdos, fat, vfat)
    Ramfs,  ///< RAMDISK mount
    Other ///< ext, reiser, and so on. "Normal" local filesystems.
};

/**
 * Returns the file system type at a given path, as much as we are able to figure it out.
 * @since 5.0
 */
KCOREADDONS_EXPORT Type fileSystemType(const QString &path);

}

#endif
