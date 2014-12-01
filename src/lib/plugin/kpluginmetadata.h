/*  This file is part of the KDE project
    Copyright (C) 2014 Alex Richardson <arichardson.kde@gmail.com>

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

#ifndef KPLUGINMETADATA_H
#define KPLUGINMETADATA_H

#include "kcoreaddons_export.h"

#include <QExplicitlySharedDataPointer>
#include <QJsonObject>
#include <QString>
#include <QVector>

#include <functional>

class KPluginLoader;
class QPluginLoader;
class QStringList;
class KPluginMetaDataPrivate;
class KAboutPerson;
class QObject;


/** This class allows easily accessing some standardized values from the JSON metadata that
 * can be embedded into Qt plugins. Additional plugin-specific metadata can be retrieved by
 * directly reading from the QJsonObject returned by KPluginMetaData::rawData().
 *
 * This class can be used instead of KPluginInfo from KService for applications that only load
 * Qt C++ plugins.
 *
 * The following keys will be read from an object "KPlugin" inside the metadata JSON:
 *
 * Key                | Accessor function    | JSON Type
 * -------------------| -------------------- | ---------------------
 * Name               | name()               | string
 * Description        | description()        | string
 * Icon               | iconName()           | string
 * Authors            | authors()            | object / object array
 * Category           | category()           | string
 * License            | license()            | string
 * Id                 | pluginId()           | string
 * Version            | version()            | string
 * Websites           | websites()           | string
 * EnabledByDefault   | isEnabledByDefault() | bool
 * ServiceTypes       | serviceTypes()       | string / string array
 *
 * The Authors key is a single object or a list of objects that have the following keys that
 * correspond to KAboutPerson properties:
 *
 * Key                | Accessor
 * -------------------| ----------------------------
 * Name               | KAboutPerson::name()
 * Email              | KAboutPerson::emailAddress()
 * Task               | KAboutPerson::task()
 * Website            | KAboutPerson::webAddress()
 * UserName           | KAboutPerson::ocsUsername()
 *
 * An example metadata json file could look like this:
 * @verbatim
   {
     "KPlugin": {
        "Name": "Date and Time",
        "Description": "Date and time by timezone",
        "Icon": "preferences-system-time",
        "Authors": { "Name": "Aaron Seigo", "Email": "aseigo@kde.org" },
        "Category": "Date and Time",
        "Dependencies": [],
        "EnabledByDefault": "true",
        "License": "LGPL",
        "Id": "time",
        "Version": "1.0",
        "Website": "http://plasma.kde.org/",
        "ServiceTypes": ["Plasma/DataEngine"]
     }
   }
   @endverbatim
 *
 * @since 5.1
 */
class KCOREADDONS_EXPORT KPluginMetaData
{
public:

    /** Creates an invalid KPluginMetaData instance */
    KPluginMetaData();

    /**
     * Reads the plugin metadata from a KPluginLoader instance. You must call setFileName()
     * or use the appropriate constructor on @p loader before calling this.
     */
    KPluginMetaData(const KPluginLoader &loader);

    /**
     * Reads the plugin metadata from a QPluginLoader instance. You must call setFileName()
     * or use the appropriate constructor on @p loader before calling this.
     */
    KPluginMetaData(const QPluginLoader &loader);

    /**
     * Reads the plugin metadata from a plugin or .desktop which can be loaded from @p file.
     *
     * For plugins, platform-specific library suffixes may be omitted since @p file will be resolved
     * using the same logic as QPluginLoader.
     *
     * If the file name ends with ".desktop", the .desktop file will be parsed instead of
     * reading the metadata from the QPluginLoader.
     *
     * @see QPluginLoader::setFileName()
     */
    KPluginMetaData(const QString &file);

    /**
     * Create a KPluginMetaData from a QJsonObject with the metadata and a file name
     * This can be used if the data is not retrieved from a Qt C++ plugin library but from some
     * other source.
     */
    KPluginMetaData(const QJsonObject &metaData, const QString &file);

    /**
     * Copy contructor
     */
    KPluginMetaData(const KPluginMetaData &);
    /**
     * Copy assignment
     */
    KPluginMetaData &operator=(const KPluginMetaData &);
    /**
     * Destructor
     */
    ~KPluginMetaData();

    /**
     * @return whether this object holds valid information about a plugin.
     * If this is @c true pluginId() will return a non-empty string.
     */
    bool isValid() const;

    /**
     * @return the absolute path to the plugin. This string can be passed to the KPluginLoader
     * or QPluginLoader constructors in order to load this plugin.
     */
    QString fileName() const;

    /**
     * @return the full metadata stored inside the plugin file.
     */
    QJsonObject rawData() const;


    /**
     * Tries to instantiate this plugin using KPluginMetaData::fileName().
     * @note The value of KPluginMetaData::dependencies() is not used here, dependencies must be
     * resolved manually.
     *
     * @return The plugin root object or @c nullptr if it could not be loaded
     * @see QPluginLoader::instance(), KPluginLoader::instance()
     */
    QObject *instantiate() const;

    /**
     * @return the user visible name of the plugin.
     */
    QString name() const;

    /**
     * @return a short description of the plugin.
     */
    QString description() const;

    /**
     * @return the author(s) of this plugin.
     */
    QList<KAboutPerson> authors() const;

    /**
     * @return the categories of this plugin (e.g. "playlist/skin").
     */
    QString category() const;

    /**
     * @return the icon name for this plugin
     * @see QIcon::fromTheme()
     */
    QString iconName() const;

    /**
     * @return the short license identifier (e.g. LGPL).
     * @see KAboutLicense::byKeyword() for retrieving the full license information
     */
    QString license() const;

    /**
     * @return the internal name of the plugin (for KParts Plugins this is
     * the same name as set in the .rc file). If the plugin name property is not set in
     * the metadata this will return the plugin file name without the file extension.
     */
    QString pluginId() const;

    /**
     * @return the version of the plugin.
     */
    QString version() const;

    /**
     * @return the website of the plugin.
     */
    QString website() const;

    /**
     * @return a list of plugins that this plugin depends on so that it can function properly
     * @see KJsonPluginInfo::pluginId()
     */
    QStringList dependencies() const;

    /**
     * @return a list of service types this plugin implements (e.g. "Plasma/DataEngine")
     */
    QStringList serviceTypes() const;

    /**
     * @return whether the plugin should be enabled by default.
     * This is only a recommendation, applications can ignore this value if they want to.
     */
    bool isEnabledByDefault() const;

    /**
     * @return the value for @p key from the metadata or @p defaultValue if the key does not exist
     * or the value for @p key is not of type string
     *
     * @see KPluginMetaData::rawData() if QString is not the correct type for @p key
     */
    QString value(const QString &key, const QString &defaultValue = QString()) const;

    /** @return the value for @p key inside @p jo as a string list. If the type of @p key is string, a list with containing
     * just that string will be returned, if it is an array the list will contain one entry for each array member.
     * If the key cannot be found an empty list will be returned.
     */
    static QStringList readStringList(const QJsonObject &jo, const QString &key);

    /**
     * Reads a value from @p jo but unlike QJsonObject::value() it allows different entries for each locale
     * This is done by appending the locale identifier in brackets to the key (e.g. "[de_DE]" or "[es]")
     * When looking for a key "foo" with German (Germany) locale we will first attempt to read "foo[de_DE]",
     * if that does not exist "foo[de]", finally falling back to "foo" if that also doesn't exist.
     * @return the translated value for @p key from @p jo or @p defaultValue if @p key was not found
     */
    static QJsonValue readTranslatedValue(const QJsonObject &jo, const QString &key, const QJsonValue &defaultValue = QJsonValue());

    /**
     * @return the translated value of @p key from @p jo as a string or @p defaultValue if @p key was not found
     * or the value for @p key is not of type string
     * @see KPluginMetaData::readTranslatedValue(const QJsonObject &jo, const QString &key)
     */
    static QString readTranslatedString(const QJsonObject &jo, const QString &key, const QString &defaultValue = QString());

    /**
     * @return @c true if this object is equal to @p other, otherwise @c false
     */
    bool operator==(const KPluginMetaData &other) const;

    /**
     * @return @c true if this object is not equal to @p other, otherwise @c false.
     */
    inline bool operator!=(const KPluginMetaData &other) const
    {
        return !(*this == other);
    }
private:
    QJsonObject rootObject() const;
private:
    QJsonObject m_metaData;
    QString m_fileName;
    QExplicitlySharedDataPointer<KPluginMetaDataPrivate> d; // for future binary compatible extensions
};

inline uint qHash(const KPluginMetaData &md, uint seed)
{
    return qHash(md.pluginId(), seed);
}

#endif // KPLUGINMETADATA_H
