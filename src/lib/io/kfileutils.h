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
 * Given a directory path and a filename (which usually exists already),
 * this function returns a suggested name for a file that doesn't exist
 * in that directory. The existence is only checked for local urls though.
 * The suggested file name is of the form "foo 1", "foo 2" etc.
 * @since 5.61
 */
KCOREADDONS_EXPORT QString suggestName(const QUrl &baseURL, const QString &oldName);

}
#endif
