/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KFILEUTILS_H
#define KFILEUTILS_H

#include "kcoreaddons_export.h"

#include <QString>
#include <QUrl>

/**
 * @short A namespace for KFileUtils globals
 *
 */
namespace KFileUtils
{

/**
 * Given a directory path and a string representing a file or directory
 * (which usually exist already), this function returns a suggested name
 * for a file/directory that doesn't exist in @p baseURL.
 *
 * The suggested file name is of the form "foo (1)", "foo (2)" etc.
 *
 * For local URLs, this function will check if there is already a file/directory
 * with the new suggested name and will keep incrementing the number in the above
 * format until it finds one that doesn't exist. Note that this function uses a
 * blocking I/O call (using QFileInfo) to check the existence of the file/directory,
 * this could be problematic for network mounts (e.g. SMB, NFS) as these are treated
 * as local files by the upstream QFile code. An alternative is to use makeSuggestedName()
 * and use KIO to stat the new file/directory in an asynchronous way.
 *
 * @since 5.61
 */
KCOREADDONS_EXPORT QString suggestName(const QUrl &baseURL, const QString &oldName);

/**
 * Given a string, "foo", representing a file/directory (which usually exists already),
 * this function returns a suggested name for a file/directory in the form "foo (1)",
 * "foo (2)" etc.
 *
 * Unlike the suggestName() method, this function doesn't check if there is a file/directory
 * with the newly suggested name; the idea being that this responsibility falls on
 * the caller, e.g. one can use KIO::stat() to check asynchronously whether the new
 * name already exists (in its parent directory) or not.
 *
 * @since 5.76
 */
KCOREADDONS_EXPORT QString makeSuggestedName(const QString &oldName);

}
#endif
