/*  This file is part of the KDE Frameworks

    Copyright 2014 David Faure <faure@kde.org>
    Copyright 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kdelibs4migration.h"
#include "config-kde4home.h"
#include <QDir>
#include <QDebug>
#include <QVector>

class Kdelibs4MigrationPrivate
{
public:
    QString m_kdeHome;
};

Kdelibs4Migration::Kdelibs4Migration()
    : d(new Kdelibs4MigrationPrivate)
{
    if (qEnvironmentVariableIsSet("KDEHOME")) {
        //qDebug() << "Using KDEHOME as the location of the old config file";
        d->m_kdeHome = QString::fromLocal8Bit(qgetenv("KDEHOME"));
    } else {
        QDir homeDir = QDir::home();
        QVector<QString> testSubdirs;
        testSubdirs << QStringLiteral(KDE4_DEFAULT_HOME) << QStringLiteral(".kde4") << QStringLiteral(".kde");
        Q_FOREACH (const QString &testSubdir, testSubdirs) {
            if (homeDir.exists(testSubdir)) {
                //qDebug() << "Using" << testSubdir << "as the location of the old config file";
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

Kdelibs4Migration::~Kdelibs4Migration()
{
    delete d;
}

bool Kdelibs4Migration::kdeHomeFound() const
{
    return !d->m_kdeHome.isEmpty() && QDir(d->m_kdeHome).exists();
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
} s_subdirs[] = {
    { "config", "share/config/" },
    { "data", "share/apps/" },
    { "services", "share/kde4/services" },
    { "servicetypes", "share/kde4/servicetypes" },
    { "wallpaper", "share/wallpapers" },
    { "emoticons", "share/emoticons" },
    { "templates", "share/templates" }
};

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
    qWarning() << "No such resource" << type;
    return QString();
}

