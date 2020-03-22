/*
    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2014 Montel Laurent <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDELIBS4CONFIGMIGRATOR_H
#define KDELIBS4CONFIGMIGRATOR_H

#include <kcoreaddons_export.h>
#include <QStringList>

/**
 * @class Kdelibs4ConfigMigrator kdelibs4configmigrator.h Kdelibs4ConfigMigrator
 *
 * Kdelibs4ConfigMigrator migrates selected config files and ui files
 * from the kdelibs 4.x location ($KDEHOME, as used by KStandardDirs)
 * to the Qt 5.x location ($XDG_*_HOME, as used by QStandardPaths).
 *
 * @short Class for migration of config files and ui file from kdelibs4.
 * @since 5.2
 */
class KCOREADDONS_EXPORT Kdelibs4ConfigMigrator
{
public:
    /**
     * Constructs a Kdelibs4ConfigMigrator
     *
     * @param appName The application name, which is used for the directory
     * containing the .ui files.
     */
    explicit Kdelibs4ConfigMigrator(const QString &appName);

    /**
     * Destructor
     */
    ~Kdelibs4ConfigMigrator();

    Kdelibs4ConfigMigrator(const Kdelibs4ConfigMigrator &) = delete;
    Kdelibs4ConfigMigrator &operator=(const Kdelibs4ConfigMigrator &) = delete;

    /**
     * Migrate the files, if any.
     *
     * Returns true if the migration happened.
     * It will return false if there was nothing to migrate (no KDEHOME).
     * This return value is unrelated to error handling. It is just a way to skip anything else
     * related to migration on a clean system, by writing
     * @code
     * if (migrate()) {
     *    look for old data to migrate as well
     * }
     * @endcode
     */
    bool migrate();

    /**
     * Set the list of config files that need to be migrated.
     * @param configFileNameList list of config files
     */
    void setConfigFiles(const QStringList &configFileNameList);

    /**
     * Set the list of ui files to migrate.
     * @param uiFileNameList list of ui files
     */
    void setUiFiles(const QStringList &uiFileNameList);

private:
    class Private;
    friend class Private;
    Private *const d;
};

#endif // KDELIBS4CONFIGMIGRATOR_H
