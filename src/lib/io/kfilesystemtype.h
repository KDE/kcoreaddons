/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2011 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only
*/

#ifndef KFILESYSTEMTYPE_P_H
#define KFILESYSTEMTYPE_P_H

#include <kcoreaddons_export.h>

#include <QString>

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
    Fat, ///< FAT or similar (msdos, FAT, VFAT)
    Ramfs, ///< RAMDISK mount
    Other, ///< Ext3, Ext4, ReiserFs, and so on. "Normal" local filesystems.
    Ntfs, ///< NTFS filesystem @since 5.85
    Exfat, ///< ExFat filesystem @since 5.86
    /**
     * FUSE (Filesystem in USErspace), this is used for a variety of mounts,
     * e.g. a network mount of a remote filesystem.
     *
     * @since 5.99
     */
    FuseBlk,
    /**
     * FUSE (Filesystem in USErspace) on a block device, e.g. FUSE used for a
     * partition on a local disk.
     *
     * A block device in this context means a filesystem managed by the sd driver
     * (driver for SCSI disk drives in the Linux kernel), so the device node is
     * e.g. /dev/sde1.
     *
     * This is the case for NTFS partitions mounted with UDisks2 (which is the
     * default backend used on most Linux distributions).
     *
     * The difference between this enumerator and FuseBlk is that the latter is
     * usually considered a network (i.e. slow) filesystem, whereas FuseBlk_BlockDevice
     * is typically a regualr/local filesystem. This makes a difference in
     * how KIO handles the filesystem.
     *
     * @since 5.99
     */
    FuseBlk_BlockDevice,
};

/**
 * For a given @p path, returns the filesystem type, one of @ref KFileSystemType::Type
 * values. If the type can't be determined, @c KFileSystemType::Unknown is returned.
 *
 * @since 5.0
 */
KCOREADDONS_EXPORT Type fileSystemType(const QString &path);

/**
 * Returns the possibly translated name of a filesystem corresponding to a
 * value from @ref KFileSystemType::Type.
 *
 * @since 5.86
 */
KCOREADDONS_EXPORT QString fileSystemName(KFileSystemType::Type type);
}

#endif
