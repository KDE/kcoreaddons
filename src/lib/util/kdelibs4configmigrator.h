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

#ifndef KDELIBS4CONFIGMIGRATOR_H
#define KDELIBS4CONFIGMIGRATOR_H

#include <kcoreaddons_export.h>
#include <QStringList>

/**
 * \file kdelibs4configmigrator.h
 */

/**
  * Kdelibs4ConfigMigrator migrates specific config file and ui file
  * from KDE SC 4.0 to new QStandardPath.
  *
  * @short Class for migration of config files and ui file from KDE SC4
  * @since 5.2
  */

class KCOREADDONS_EXPORT Kdelibs4ConfigMigrator
{
public:
    /**
     * Constructs a Kdelibs4ConfigMigrator
     *
     * @param appName The application name of KDE SC 4.0
     */
    explicit Kdelibs4ConfigMigrator(const QString &appName);

    /**
     * Destructor
     */
    ~Kdelibs4ConfigMigrator();

    /**
     * Return true if migrate was done. If we found kdehome directory
     */
    bool migrate();

    /**
     * Set list of config files we need to migrate for application
     * @param configFileNameList list of config file
     */
    void setConfigFiles(const QStringList &configFileNameList);

    /**
     * Set list of ui files to migrate
     * @param uiFileNameList list of ui file.
     */
    void setUiFiles(const QStringList &uiFileNameList);

private:
    class Private;
    friend class Private;
    Private *const d;
};

#endif // KDELIBS4CONFIGMIGRATOR_H
