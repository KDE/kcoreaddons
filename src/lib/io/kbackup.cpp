/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2006 Allen Winter <winter@kde.org>
    SPDX-FileCopyrightText: 2006 Gregory S. Hayes <syncomm@kde.org>
    SPDX-FileCopyrightText: 2006 Jaison Lee <lee.jaison@gmail.com>
    SPDX-FileCopyrightText: 2011 Romain Perier <bambi@ubuntu.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbackup.h"

#include <QDir>
#include <QFileInfo>

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 75)
#include <QProcess>
#include <QStandardPaths>
#endif

namespace KBackup
{
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 75)
bool backupFile(const QString &qFilename, const QString &backupDir)
{
    return (simpleBackupFile(qFilename, backupDir, QStringLiteral("~")));
}
#endif

bool simpleBackupFile(const QString &qFilename, const QString &backupDir, const QString &backupExtension)
{
    QString backupFileName = qFilename + backupExtension;

    if (!backupDir.isEmpty()) {
        QFileInfo fileInfo(qFilename);
        backupFileName = backupDir + QLatin1Char('/') + fileInfo.fileName() + backupExtension;
    }

    //    qCDebug(KCOREADDONS_DEBUG) << "KBackup copying " << qFilename << " to " << backupFileName;
    QFile::remove(backupFileName);
    return QFile::copy(qFilename, backupFileName);
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 75)
bool rcsBackupFile(const QString &qFilename, const QString &backupDir, const QString &backupMessage)
{
    QFileInfo fileInfo(qFilename);

    QString qBackupFilename;
    if (backupDir.isEmpty()) {
        qBackupFilename = qFilename;
    } else {
        qBackupFilename = backupDir + fileInfo.fileName();
    }
    qBackupFilename += QLatin1String(",v");

    // If backupDir is specified, copy qFilename to the
    // backupDir and perform the commit there, unlinking
    // backupDir/qFilename when finished.
    if (!backupDir.isEmpty()) {
        if (!QFile::copy(qFilename, backupDir + fileInfo.fileName())) {
            return false;
        }
        fileInfo.setFile(backupDir + QLatin1Char('/') + fileInfo.fileName());
    }

    const QString cipath = QStandardPaths::findExecutable(QStringLiteral("ci"));
    const QString copath = QStandardPaths::findExecutable(QStringLiteral("co"));
    const QString rcspath = QStandardPaths::findExecutable(QStringLiteral("rcs"));
    if (cipath.isEmpty() || copath.isEmpty() || rcspath.isEmpty()) {
        return false;
    }

    // Check in the file unlocked with 'ci'
    QProcess ci;
    if (!backupDir.isEmpty()) {
        ci.setWorkingDirectory(backupDir);
    }
    ci.start(cipath, QStringList{QStringLiteral("-u"), fileInfo.filePath()});
    if (!ci.waitForStarted()) {
        return false;
    }
    ci.write(backupMessage.toLocal8Bit());
    ci.write(".");
    ci.closeWriteChannel();
    if (!ci.waitForFinished()) {
        return false;
    }

    // Use 'rcs' to unset strict locking
    QProcess rcs;
    if (!backupDir.isEmpty()) {
        rcs.setWorkingDirectory(backupDir);
    }
    rcs.start(rcspath, QStringList{QStringLiteral("-U"), qBackupFilename});
    if (!rcs.waitForFinished()) {
        return false;
    }

    // Use 'co' to checkout the current revision and restore permissions
    QProcess co;
    if (!backupDir.isEmpty()) {
        co.setWorkingDirectory(backupDir);
    }
    co.start(copath, QStringList{qBackupFilename});
    if (!co.waitForFinished()) {
        return false;
    }

    if (!backupDir.isEmpty()) {
        return QFile::remove(fileInfo.filePath());
    } else {
        return true;
    }
}
#endif

bool numberedBackupFile(const QString &qFilename, const QString &backupDir, const QString &backupExtension, const uint maxBackups)
{
    QFileInfo fileInfo(qFilename);

    // The backup file name template.
    QString sTemplate;
    if (backupDir.isEmpty()) {
        sTemplate = qFilename + QLatin1String(".%1") + backupExtension;
    } else {
        sTemplate = backupDir + QLatin1Char('/') + fileInfo.fileName() + QLatin1String(".%1") + backupExtension;
    }

    // First, search backupDir for numbered backup files to remove.
    // Remove all with number 'maxBackups' and greater.
    QDir d = backupDir.isEmpty() ? fileInfo.dir() : backupDir;
    d.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    const QStringList nameFilters = QStringList(fileInfo.fileName() + QLatin1String(".*") + backupExtension);
    d.setNameFilters(nameFilters);
    d.setSorting(QDir::Name);

    uint maxBackupFound = 0;
    const QFileInfoList infoList = d.entryInfoList();
    for (const QFileInfo &fi : infoList) {
        if (fi.fileName().endsWith(backupExtension)) {
            // sTemp holds the file name, without the ending backupExtension
            QString sTemp = fi.fileName();
            sTemp.truncate(fi.fileName().length() - backupExtension.length());
            // compute the backup number
            int idex = sTemp.lastIndexOf(QLatin1Char('.'));
            if (idex > 0) {
                bool ok;
                const uint num = sTemp.midRef(idex + 1).toUInt(&ok);
                if (ok) {
                    if (num >= maxBackups) {
                        QFile::remove(fi.filePath());
                    } else {
                        maxBackupFound = qMax(maxBackupFound, num);
                    }
                }
            }
        }
    }

    // Next, rename max-1 to max, max-2 to max-1, etc.
    QString to = sTemplate.arg(maxBackupFound + 1);
    for (int i = maxBackupFound; i > 0; i--) {
        QString from = sTemplate.arg(i);
        //        qCDebug(KCOREADDONS_DEBUG) << "KBackup renaming " << from << " to " << to;
        QFile::rename(from, to);
        to = from;
    }

    // Finally create most recent backup by copying the file to backup number 1.
    //    qCDebug(KCOREADDONS_DEBUG) << "KBackup copying " << qFilename << " to " << sTemplate.arg(1);
    return QFile::copy(qFilename, sTemplate.arg(1));
}

}
