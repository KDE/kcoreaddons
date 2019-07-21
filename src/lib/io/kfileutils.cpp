/* This file is part of the KDE libraries
   Copyright (C) 2000-2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kfileutils.h"

#include <QMimeDatabase>
#include <QFileInfo>

QString KFileUtils::suggestName(const QUrl &baseURL, const QString &oldName)
{
    QString basename;

    // Extract the original file extension from the filename
    QMimeDatabase db;
    QString nameSuffix = db.suffixForFileName(oldName);

    if (oldName.lastIndexOf(QLatin1Char('.')) == 0) {
        basename = QStringLiteral(".");
        nameSuffix = oldName;
    } else if (nameSuffix.isEmpty()) {
        const int lastDot = oldName.lastIndexOf(QLatin1Char('.'));
        if (lastDot == -1) {
            basename = oldName;
        } else {
            basename = oldName.left(lastDot);
            nameSuffix = oldName.mid(lastDot);
        }
    } else {
        nameSuffix.prepend(QLatin1Char('.'));
        basename = oldName.left(oldName.length() - nameSuffix.length());
    }

    // check if (number) exists from the end of the oldName and increment that number
    QRegExp numSearch(QStringLiteral("\\(\\d+\\)"));
    int start = numSearch.lastIndexIn(oldName);
    if (start != -1) {
        QString numAsStr = numSearch.cap(0);
        QString number = QString::number(numAsStr.midRef(1, numAsStr.size() - 2).toInt() + 1);
        basename = basename.leftRef(start) + QLatin1Char('(') + number + QLatin1Char(')');
    } else {
        // number does not exist, so just append " (1)" to filename
        basename += QLatin1String(" (1)");
    }
    const QString suggestedName = basename + nameSuffix;

    // Check if suggested name already exists
    bool exists = false;

    if (baseURL.isLocalFile()) {
        exists = QFileInfo::exists(baseURL.toLocalFile() + QLatin1Char('/') + suggestedName);
    }

    if (!exists) {
        return suggestedName;
    } else { // already exists -> recurse
        return suggestName(baseURL, suggestedName);
    }
}
