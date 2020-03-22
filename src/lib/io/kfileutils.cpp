/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000-2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kfileutils.h"

#include <QMimeDatabase>
#include <QFileInfo>
#include <QRegularExpression>

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

    // check if (number) exists at the end of the oldName and increment that number
    const QRegularExpression re(QStringLiteral("\\((\\d+)\\)"));
    QRegularExpressionMatch rmatch;
    oldName.lastIndexOf(re, -1, &rmatch);
    if (rmatch.hasMatch()) {
        const int currentNum = rmatch.captured(1).toInt();
        const QString number = QString::number(currentNum + 1);
        basename.replace(rmatch.capturedStart(1), rmatch.capturedLength(1), number);
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
