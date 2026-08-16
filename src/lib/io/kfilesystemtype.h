/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2011 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only
*/

#ifndef KFILESYSTEMTYPE_P_H
#define KFILESYSTEMTYPE_P_H

#include <kcoreaddons_export.h>

#include <QString>

/*!
 * \namespace KFileSystemType
 * \inmodule KCoreAddons
 * Provides utility functions for the type of file systems.
 */
namespace KFileSystemType
{
/*!
 * \value Unknown Unknown
 * \value Nfs NFS or other full-featured networked filesystems (autofs, subfs, cachefs, sshfs)
 * \value Smb SMB/CIFS mount (networked but with some FAT-like behavior)
 * \value Fat FAT or similar (msdos, FAT, VFAT)
 * \value Ramfs RAMDISK mount
 * \value Other Ext3, Ext4, ReiserFs, and so on. "Normal" local filesystems.
 * \value [since 5.85] Ntfs NTFS filesystem
 * \value [since 5.86] Exfat ExFat filesystem
 * \value [since 5.100] Fuse (Filesystem in USErspace) on a block device, holding a filesystem this
 * machine could not name. The kernel calls this kind of mount fuseblk.
 * \value [since 6.30] FuseNoDev A fuse mount with no block device behind it, the kind
 * /proc/filesystems marks nodev, which is a different thing from the nodev mount option. The files
 * come from a program rather than from a filesystem on a disk, and that program may be serving a
 * directory that lives on another machine.
 */
enum Type {
    Unknown,
    Nfs,
    Smb,
    Fat,
    Ramfs,
    Other,
    Ntfs,
    Exfat,
    // TODO KF7: rename to FuseBlk
    Fuse,
    FuseNoDev,
};

/*!
 * For a given \a path, returns the filesystem type, one of KFileSystemType::Type
 * values. If the type can't be determined, KFileSystemType::Unknown is returned.
 *
 * \since 5.0
 */
KCOREADDONS_EXPORT Type fileSystemType(const QString &path);

/*!
 * Returns the possibly translated name of a filesystem corresponding to a
 * value from KFileSystemType::Type.
 *
 * \since 5.86
 */
KCOREADDONS_EXPORT QString fileSystemName(KFileSystemType::Type type);
}

#endif
