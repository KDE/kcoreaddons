/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdelibs4migration.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

#include "config-util.h"
#include "kcoreaddons_debug.h"
#include <QDir>
#include <QVector>

#ifdef Q_OS_WIN
#include <shlobj.h>
#endif

class Kdelibs4MigrationPrivate
{
public:
    QString m_kdeHome;
};

Kdelibs4Migration::Kdelibs4Migration()
    : d(new Kdelibs4MigrationPrivate)
{
    if (qEnvironmentVariableIsSet("KDEHOME")) {
        qCDebug(KCOREADDONS_DEBUG) << "Using KDEHOME as the location of the old config file";
        d->m_kdeHome = QString::fromLocal8Bit(qgetenv("KDEHOME"));
    } else {
        QDir homeDir = QDir::home();
        QVector<QString> testSubdirs;
        testSubdirs << QStringLiteral(KDE4_DEFAULT_HOME) << QStringLiteral(".kde4") << QStringLiteral(".kde");
#ifdef Q_OS_WIN
        WCHAR wPath[MAX_PATH + 1];
        if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wPath) == S_OK) {
            testSubdirs << QDir::fromNativeSeparators(QString::fromUtf16((const ushort *)wPath)) + QLatin1String("/" KDE4_DEFAULT_HOME);
        }
#endif
        for (const QString &testSubdir : std::as_const(testSubdirs)) {
            if (homeDir.exists(testSubdir)) {
                qCDebug(KCOREADDONS_DEBUG) << "Using" << testSubdir << "as the location of the old config file";
                d->m_kdeHome = homeDir.filePath(testSubdir);
                break;
            }
        }
        if (d->m_kdeHome.isEmpty()) {
            d->m_kdeHome = homeDir.filePath(QStringLiteral(KDE4_DEFAULT_HOME));
        }
    }

    if (!d->m_kdeHome.isEmpty() && !d->m_kdeHome.endsWith(QLatin1Char('/'))) {
        d->m_kdeHome.append(QLatin1Char('/'));
    }
}

Kdelibs4Migration::~Kdelibs4Migration() = default;

bool Kdelibs4Migration::kdeHomeFound() const
{
    return !d->m_kdeHome.isEmpty() && QDir(d->m_kdeHome).exists();
}

QString Kdelibs4Migration::kdeHome() const
{
    return d->m_kdeHome;
}

QString Kdelibs4Migration::locateLocal(const char *type, const QString &filename) const
{
    if (d->m_kdeHome.isEmpty()) {
        return QString();
    }
    const QString dir = saveLocation(type);
    if (dir.isEmpty()) {
        return QString();
    }
    const QString file = dir + filename;
    if (QFile::exists(file)) {
        return file;
    }
    return QString();
}

static const struct {
    const char *type;
    const char *subdir;
} s_subdirs[] = {{"config", "share/config/"},
                 {"data", "share/apps/"},
                 {"services", "share/kde4/services"},
                 {"servicetypes", "share/kde4/servicetypes"},
                 {"wallpaper", "share/wallpapers"},
                 {"emoticons", "share/emoticons"},
                 {"templates", "share/templates"}};

QString Kdelibs4Migration::saveLocation(const char *type, const QString &suffix) const
{
    if (d->m_kdeHome.isEmpty()) {
        return QString();
    }
    static const int numResources = sizeof(s_subdirs) / sizeof(*s_subdirs);
    for (uint i = 0; i < numResources; ++i) {
        if (qstrcmp(s_subdirs[i].type, type) == 0) {
            QString dir = d->m_kdeHome + QString::fromLatin1(s_subdirs[i].subdir) + suffix;
            if (!dir.endsWith(QLatin1Char('/'))) {
                dir += QLatin1Char('/');
            }
            return dir;
        }
    }
    qCWarning(KCOREADDONS_DEBUG) << "No such resource" << type;
    return QString();
}
#endif
