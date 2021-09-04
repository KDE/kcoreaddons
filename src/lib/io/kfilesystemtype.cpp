/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2011 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only
*/

#include "kfilesystemtype.h"
#include "kcoreaddons_debug.h"
#include "knetworkmounts.h"

#include <QCoreApplication>
#include <QFile>

#ifndef Q_OS_WIN
inline KFileSystemType::Type kde_typeFromName(const char *name)
{
    /* clang-format off */
    if (qstrncmp(name, "nfs", 3) == 0
        || qstrncmp(name, "autofs", 6) == 0
        || qstrncmp(name, "cachefs", 7) == 0
        || qstrncmp(name, "fuse.sshfs", 10) == 0
        || qstrncmp(name, "xtreemfs@", 9) == 0) { // #178678

        return KFileSystemType::Nfs;
    }
    if (qstrncmp(name, "fat", 3) == 0
        || qstrncmp(name, "vfat", 4) == 0
        || qstrncmp(name, "msdos", 5) == 0) {
        return KFileSystemType::Fat;
    }
    if (qstrncmp(name, "cifs", 4) == 0
        || qstrncmp(name, "smbfs", 5) == 0) {
        return KFileSystemType::Smb;
    }
    if (qstrncmp(name, "ramfs", 5) == 0) {
        return KFileSystemType::Ramfs;
    }
    /* clang-format on */

    return KFileSystemType::Other;
}

#if defined(Q_OS_BSD4) && !defined(Q_OS_NETBSD)
#include <sys/mount.h>
#include <sys/param.h>

KFileSystemType::Type determineFileSystemTypeImpl(const QByteArray &path)
{
    struct statfs buf;
    if (statfs(path.constData(), &buf) != 0) {
        return KFileSystemType::Unknown;
    }
    return kde_typeFromName(buf.f_fstypename);
}

#elif defined(Q_OS_LINUX) || defined(Q_OS_HURD)
#include <sys/statfs.h>

#ifdef Q_OS_LINUX
#include <linux/magic.h> // A lot of the filesystem superblock MAGIC numbers
#endif

// From /usr/src/linux-5.13.2-1-vanilla/fs/ntfs/ntfs.h
#ifndef NTFS_SB_MAGIC
#define NTFS_SB_MAGIC 0x5346544e
#endif

// From /usr/src/linux-5.13.2-1-vanilla/fs/exfat/exfat_fs.h
#ifndef EXFAT_SUPER_MAGIC
#define EXFAT_SUPER_MAGIC 0x2011BAB0UL
#endif

// From /usr/src/linux-5.13.2-1-vanilla/fs/cifs/smb2glob.h
#ifndef SMB2_MAGIC_NUMBER
#define SMB2_MAGIC_NUMBER 0xFE534D42
#endif

// From /usr/src/linux-5.13.2-1-vanilla/fs/cifs/cifsglob.h
#ifndef CIFS_MAGIC_NUMBER
#define CIFS_MAGIC_NUMBER 0xFF534D42
#endif

// From /usr/src/linux-5.13.2-1-vanilla/fs/fuse/inode.c
#ifndef FUSE_SUPER_MAGIC
#define FUSE_SUPER_MAGIC 0x65735546
#endif

#ifndef AUTOFSNG_SUPER_MAGIC
#define AUTOFSNG_SUPER_MAGIC 0x7d92b1a0
#endif

#ifdef Q_OS_HURD
#ifndef NFS_SUPER_MAGIC
#define NFS_SUPER_MAGIC 0x00006969
#endif
#ifndef AUTOFS_SUPER_MAGIC
#define AUTOFS_SUPER_MAGIC 0x00000187
#endif
#ifndef MSDOS_SUPER_MAGIC
#define MSDOS_SUPER_MAGIC 0x00004d44
#endif
#ifndef SMB_SUPER_MAGIC
#define SMB_SUPER_MAGIC 0x0000517B
#endif
#ifndef RAMFS_MAGIC
#define RAMFS_MAGIC 0x858458F6
#endif
#endif

// Reverse-engineering without C++ code:
// strace stat -f /mnt 2>&1|grep statfs|grep mnt, and look for f_type
//
// Also grep'ing in /usr/src/<kernel-version>/fs/

static KFileSystemType::Type determineFileSystemTypeImpl(const QByteArray &path)
{
    struct statfs buf;
    if (statfs(path.constData(), &buf) != 0) {
        return KFileSystemType::Unknown;
    }

    switch (static_cast<unsigned long>(buf.f_type)) {
    case NFS_SUPER_MAGIC:
    case AUTOFS_SUPER_MAGIC:
    case AUTOFSNG_SUPER_MAGIC:
    case FUSE_SUPER_MAGIC: // TODO could be anything. Need to use statfs() to find out more.
        return KFileSystemType::Nfs;
    case SMB_SUPER_MAGIC:
    case SMB2_MAGIC_NUMBER:
    case CIFS_MAGIC_NUMBER:
        return KFileSystemType::Smb;
    case MSDOS_SUPER_MAGIC:
        return KFileSystemType::Fat;
    case NTFS_SB_MAGIC:
        return KFileSystemType::Ntfs;
    case EXFAT_SUPER_MAGIC:
        return KFileSystemType::Exfat;
    case RAMFS_MAGIC:
        return KFileSystemType::Ramfs;
    default:
        return KFileSystemType::Other;
    }
}

#elif defined(Q_OS_AIX) || defined(Q_OS_HPUX) || defined(Q_OS_QNX) || defined(Q_OS_SCO) || defined(Q_OS_UNIXWARE) || defined(Q_OS_RELIANT)                     \
    || defined(Q_OS_NETBSD)
#include <sys/statvfs.h>

KFileSystemType::Type determineFileSystemTypeImpl(const QByteArray &path)
{
    struct statvfs buf;
    if (statvfs(path.constData(), &buf) != 0) {
        return KFileSystemType::Unknown;
    }
#if defined(Q_OS_NETBSD)
    return kde_typeFromName(buf.f_fstypename);
#else
    return kde_typeFromName(buf.f_basetype);
#endif
}
#endif
#else
KFileSystemType::Type determineFileSystemTypeImpl(const QByteArray &path)
{
    return KFileSystemType::Unknown;
}
#endif

KFileSystemType::Type KFileSystemType::fileSystemType(const QString &path)
{
    if (KNetworkMounts::self()->isSlowPath(path, KNetworkMounts::KNetworkMountsType::SmbPaths)) {
        return KFileSystemType::Smb;
    } else if (KNetworkMounts::self()->isSlowPath(path, KNetworkMounts::KNetworkMountsType::NfsPaths)) {
        return KFileSystemType::Nfs;
    } else {
        return determineFileSystemTypeImpl(QFile::encodeName(path));
    }
}

QString KFileSystemType::fileSystemName(KFileSystemType::Type type)
{
    switch (type) {
    case KFileSystemType::Nfs:
        return QCoreApplication::translate("KFileSystemType", "NFS");
    case KFileSystemType::Smb:
        return QCoreApplication::translate("KFileSystemType", "SMB");
    case KFileSystemType::Fat:
        return QCoreApplication::translate("KFileSystemType", "FAT");
    case KFileSystemType::Ramfs:
        return QCoreApplication::translate("KFileSystemType", "RAMFS");
    case KFileSystemType::Other:
        return QCoreApplication::translate("KFileSystemType", "Other");
    case KFileSystemType::Ntfs:
        return QCoreApplication::translate("KFileSystemType", "NTFS");
    case KFileSystemType::Exfat:
        return QCoreApplication::translate("KFileSystemType", "ExFAT");
    case KFileSystemType::Unknown:
        return QCoreApplication::translate("KFileSystemType", "Unknown");
    }

    Q_UNREACHABLE();
    return {};
}
