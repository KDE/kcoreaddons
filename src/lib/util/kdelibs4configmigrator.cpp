/*  This file is part of the KDE Frameworks

    Copyright 2014 Montel Laurent <montel@kde.org>

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

#include "kdelibs4configmigrator.h"

#include <QStandardPaths>
#include <Kdelibs4Migration>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QLoggingCategory>
#include <QPluginLoader>

Q_DECLARE_LOGGING_CATEGORY(MIGRATOR)
// logging category for this framework, default: log stuff >= warning
Q_LOGGING_CATEGORY(MIGRATOR, "kf5.kcoreaddons.kdelibs4configmigrator", QtWarningMsg)

class Q_DECL_HIDDEN Kdelibs4ConfigMigrator::Private
{
public:
    Private(const QString &_appName)
        : appName(_appName)
    {

    }

    QStringList configFiles;
    QStringList uiFiles;
    QString appName;
};

Kdelibs4ConfigMigrator::Kdelibs4ConfigMigrator(const QString &appName)
    : d(new Private(appName))
{
}

Kdelibs4ConfigMigrator::~Kdelibs4ConfigMigrator()
{
    delete d;
}

void Kdelibs4ConfigMigrator::setConfigFiles(const QStringList &configFileNameList)
{
    d->configFiles = configFileNameList;
}

void Kdelibs4ConfigMigrator::setUiFiles(const QStringList &uiFileNameList)
{
    d->uiFiles = uiFileNameList;
}

bool Kdelibs4ConfigMigrator::migrate()
{
    // Testing for kdehome
    Kdelibs4Migration migration;
    if (!migration.kdeHomeFound()) {
        return false;
    }

    bool didSomething = false;

    for (const QString &configFileName : qAsConst(d->configFiles)) {
        const QString newConfigLocation
            = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
              + QLatin1Char('/') + configFileName;

        if (QFile(newConfigLocation).exists()) {
            continue;
        }
        //Be safe
        QFileInfo fileInfo(newConfigLocation);
        QDir().mkpath(fileInfo.absolutePath());

        const QString oldConfigFile(migration.locateLocal("config", configFileName));
        if (!oldConfigFile.isEmpty()) {
            if (QFile(oldConfigFile).copy(newConfigLocation)) {
                didSomething = true;
                qCDebug(MIGRATOR) << "config file" << oldConfigFile << "was migrated to" << newConfigLocation;
            }
        }
    }

    if (d->appName.isEmpty() && !d->uiFiles.isEmpty()) {
        qCritical() << " We can not migrate ui file. AppName is missing";
    } else {
        for (const QString &uiFileName : qAsConst(d->uiFiles)) {
            const QString newConfigLocation
                = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                  QLatin1String("/kxmlgui5/") + d->appName + QLatin1Char('/') + uiFileName;
            if (QFile(newConfigLocation).exists()) {
                continue;
            }
            QFileInfo fileInfo(newConfigLocation);
            QDir().mkpath(fileInfo.absolutePath());

            const QString oldConfigFile(migration.locateLocal("data", d->appName + QLatin1Char('/') + uiFileName));
            if (!oldConfigFile.isEmpty()) {
                if (QFile(oldConfigFile).copy(newConfigLocation)) {
                    didSomething = true;
                    qCDebug(MIGRATOR) << "ui file" << oldConfigFile << "was migrated to" << newConfigLocation;
                }
            }
        }
    }

    // Trigger KSharedConfig::openConfig()->reparseConfiguration() via the framework integration plugin
    if (didSomething) {
        QPluginLoader lib(QStringLiteral("kf5/FrameworkIntegrationPlugin"));
        QObject *rootObj = lib.instance();
        if (rootObj) {
            QMetaObject::invokeMethod(rootObj, "reparseConfiguration");
        }
    }

    return true;
}
