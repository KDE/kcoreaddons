/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINMETADATA_H
#define KPLUGINMETADATA_H

#include "kcoreaddons_export.h"

#include <QExplicitlySharedDataPointer>
#include <QJsonObject>
#include <QMetaType>
#include <QString>
#include <QStringList>

#include <functional>

class KPluginLoader;
class KPluginFactory;
class QPluginLoader;
class KPluginMetaDataPrivate;
class KAboutPerson;
class QObject;
class QStaticPlugin;
/**
 * @class KPluginMetaData kpluginmetadata.h KPluginMetaData
 *
 * This class allows easily accessing some standardized values from the JSON metadata that
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
 * ExtraInformation   | extraInformation()   | string
 * Icon               | iconName()           | string
 * Authors            | authors()            | object array (KAboutPerson)
 * Category           | category()           | string
 * License            | license()            | string
 * Copyright          | copyrightText()      | string
 * Id                 | pluginId()           | string
 * Version            | version()            | string
 * Website            | website()            | string
 * BugReportUrl       | bugReportUrl()       | string
 * EnabledByDefault   | isEnabledByDefault() | bool
 * ServiceTypes       | serviceTypes()       | string array
 * MimeTypes          | mimeTypes()          | string array
 * FormFactors        | formFactors()        | string array
 * Translators        | translators()        | object array (KAboutPerson)
 * OtherContributors  | otherContributors()  | object array (KAboutPerson)
 *
 * The Authors, Translators and OtherContributors keys are expected to be
 * list of objects that match the structure expected by KAboutPerson::fromJSON().
 *
 * An example metadata json file could look like this:
 * @verbatim
   {
     "KPlugin": {
        "Name": "Date and Time",
        "Description": "Date and time by timezone",
        "Icon": "preferences-system-time",
        "Authors": [ { "Name": "Aaron Seigo", "Email": "aseigo@kde.org" } ],
        "Category": "Date and Time",
        "EnabledByDefault": "true",
        "License": "LGPL",
        "Id": "time",
        "Version": "1.0",
        "Website": "https://plasma.kde.org/"
     }
   }
   @endverbatim
 *
 * @sa KAboutPerson::fromJSON()
 * @since 5.1
 */
class KCOREADDONS_EXPORT KPluginMetaData
{
    Q_GADGET
    Q_PROPERTY(bool isValid READ isValid CONSTANT)
    Q_PROPERTY(bool isHidden READ isHidden CONSTANT)
    Q_PROPERTY(QString fileName READ fileName CONSTANT)
    Q_PROPERTY(QString metaDataFileName READ metaDataFileName CONSTANT)
    Q_PROPERTY(QJsonObject rawData READ rawData CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 87)
    Q_PROPERTY(QString extraInformation READ extraInformation CONSTANT)
#endif
    Q_PROPERTY(QVariantList authors READ authorsVariant CONSTANT)
    Q_PROPERTY(QVariantList translators READ translatorsVariant CONSTANT)
    Q_PROPERTY(QVariantList otherContributors READ otherContributorsVariant CONSTANT)
    Q_PROPERTY(QString category READ category CONSTANT)
    Q_PROPERTY(QString iconName READ iconName CONSTANT)
    Q_PROPERTY(QString license READ license CONSTANT)
    Q_PROPERTY(QString licenseText READ licenseText CONSTANT)
    Q_PROPERTY(QString copyrightText READ copyrightText CONSTANT)
    Q_PROPERTY(QString pluginId READ pluginId CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString website READ website CONSTANT)
    Q_PROPERTY(QString bugReportUrl READ bugReportUrl CONSTANT)
#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 79)
    Q_PROPERTY(QStringList dependencies READ dependencies CONSTANT)
#endif
#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 89)
    Q_PROPERTY(QStringList serviceTypes READ serviceTypes CONSTANT)
#endif
    Q_PROPERTY(QStringList mimeTypes READ mimeTypes CONSTANT)
    Q_PROPERTY(QStringList formFactors READ formFactors CONSTANT)
    Q_PROPERTY(bool isEnabledByDefault READ isEnabledByDefault CONSTANT)
#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 104)
    Q_PROPERTY(int initialPreference READ isEnabledByDefault CONSTANT)
#endif

public:
    /**
     * Options for creating a KPluginMetaData object.
     * @since 5.91
     */
    enum KPluginMetaDataOption {
        DoNotAllowEmptyMetaData, /// Plugins with empty metaData are considered invalid
        AllowEmptyMetaData, /// Plugins with empty metaData are considered valid
    };

    /** Creates an invalid KPluginMetaData instance */
    KPluginMetaData();

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 86)
    /**
     * Reads the plugin metadata from a KPluginLoader instance. You must call KPluginLoader::setFileName()
     * or use the appropriate constructor on @p loader before calling this.
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use KPluginMetaData(QPluginLoader) instead")
    KPluginMetaData(const KPluginLoader &loader);
#endif

    /**
     * Reads the plugin metadata from a QPluginLoader instance. You must call QPluginLoader::setFileName()
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
     * reading the metadata from the QPluginLoader. This is the same as calling
     * KPluginMetaData::fromDesktopFile() without the serviceTypes parameter.
     *
     * If @p file ends with .json, the file will be loaded as the QJsonObject metadata.
     *
     * @note Using this constructor for metadata files is deprecated..
     * Use KPluginMetaData::fromDesktopFile or KPluginMetaData::fromJsonFile instead.
     *
     * @see QPluginLoader::setFileName()
     * @see KPluginMetaData::fromDesktopFile()
     * @see KPluginMetaData::fromJsonFile()
     */
    KPluginMetaData(const QString &file);

    /**
     * Overload which takes an option parameter that gets used when creating the KPluginMetaData instances
     * @since 5.91
     */
    KPluginMetaData(const QString &file, KPluginMetaDataOption option);

    /**
     * Creates a KPluginMetaData from a QJsonObject holding the metadata and a file name
     * This can be used if the data is not retrieved from a Qt C++ plugin library but from some
     * other source.
     * @see KPluginMetaData(const QJsonObject &, const QString &, const QString &)
     */
    KPluginMetaData(const QJsonObject &metaData, const QString &file);

    // TODO: KF6: merge with the above and make metaDataFile default to QString()
    /**
     * Creates a KPluginMetaData
     * @param metaData the JSON metadata to use for this object
     * @param pluginFile the file that the plugin can be loaded from
     * @param metaDataFile the file that the JSON metadata was read from
     *
     * This can be used if the data is not retrieved from a Qt C++ plugin library but from some
     * other source.
     *
     * @since 5.5
     */
    KPluginMetaData(const QJsonObject &metaData, const QString &pluginFile, const QString &metaDataFile);

    /*
     * Constructs a KPluginMetaData from the static plugin.
     * If it does not have any meta data the @p metaData value is used
     *
     * @see KPluginFactory::loadFactory
     * @see KPluginFactory::instantiatePlugin
     *
     * @since 5.89
     */
    KPluginMetaData(QStaticPlugin plugin, const QJsonObject &metaData = {});

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

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 92)
    /**
     * Load a KPluginMetaData instance from a .desktop file. Unlike the constructor which takes
     * a single file parameter this method allows you to specify which service type files should
     * be parsed to determine the correct type for a given .desktop property.
     * This ensures that a e.g. comma-separated string list field in the .desktop file will correctly
     * be converted to a JSON string array.
     *
     * @note This function mostly exists for backwards-compatibility. It is recommended
     * that new applications load JSON files directly instead of using .desktop files for plugin metadata.
     *
     * @param file the .desktop file to load
     * @param serviceTypes a list of files to parse If one of these paths is a relative path it
     * will be resolved relative to the "kservicetypes5" subdirectory in QStandardPaths::GenericDataLocation.
     * If the list is empty only the default set of properties will be treated specially and all other entries
     * will be read as the JSON string type.
     *
     * @since 5.16
     * @deprecated Since 5.92, use json files or embedded json metadata directly.
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 92, "Use json files or embedded json metadata directly")
    static KPluginMetaData fromDesktopFile(const QString &file, const QStringList &serviceTypes = QStringList());
#endif

    /**
     * Load a KPluginMetaData instance from a .json file. Unlike the constructor with a single file argument,
     * this ensure that only JSON format plugins are loaded and any other type is rejected.
     *
     * @param jsonFile the .json file to load
     * @since 5.91
     */
    static KPluginMetaData fromJsonFile(const QString &jsonFile);

    /**
     * @param directory The directory to search for plugins. If a relative path is given for @p directory,
     * all entries of QCoreApplication::libraryPaths() will be checked with @p directory appended as a
     * subdirectory. If an absolute path is given only that directory will be searched.
     * @note Check if the returned KPluginMetaData is valid before continuing to use it.
     *
     * @param pluginId The Id of the plugin. The id should be the same as the filename, see KPluginMetaData::pluginId()
     * @since 5.84
     */
    static KPluginMetaData findPluginById(const QString &directory, const QString &pluginId);

    /**
     * Find all plugins inside @p directory. Only plugins which have JSON metadata will be considered.
     *
     * @param directory The directory to search for plugins. If a relative path is given for @p directory,
     * all entries of QCoreApplication::libraryPaths() will be checked with @p directory appended as a
     * subdirectory. If an absolute path is given only that directory will be searched.
     *
     * @param filter a callback function that returns @c true if the found plugin should be loaded
     * and @c false if it should be skipped. If this argument is omitted all plugins will be loaded
     *
     * @return all plugins found in @p directory that fulfil the constraints of @p filter
     * @since 5.86
     */
    static QVector<KPluginMetaData> findPlugins(const QString &directory, std::function<bool(const KPluginMetaData &)> filter = {});

    /**
     * @since 5.91
     */
    static QVector<KPluginMetaData> findPlugins(const QString &directory, std::function<bool(const KPluginMetaData &)> filter, KPluginMetaDataOption option);

    /**
     * @return whether this object holds valid information about a plugin.
     * If this is @c true pluginId() will return a non-empty string.
     */
    bool isValid() const;

    /**
     * @return whether this object should be hidden, this is usually not used for binary
     * plugins, when loading a KPluginMetaData from a .desktop file, this will reflect
     * the value of the "Hidden" key.
     *
     * @since 5.8
     */
    bool isHidden() const;

    /**
     * @return the path to the plugin. This string can be passed to the KPluginLoader
     * or QPluginLoader constructors in order to attempt to load this plugin.
     * @note It is not guaranteed that this is a valid path to a shared library (i.e. loadable
     * by QPluginLoader) since the metadata could also refer to a non-C++ plugin.
     */
    QString fileName() const;

    /**
     * @return the file that the metadata was read from. This is not necessarily the same as
     * fileName(), since not all plugins have the metadata embedded. The metadata could also be
     * stored in a separate .desktop file.
     *
     * @since 5.5
     */
    QString metaDataFileName() const;

    /**
     * @return the full metadata stored inside the plugin file.
     */
    QJsonObject rawData() const;

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 86)
    /**
     * Tries to instantiate this plugin using KPluginMetaData::fileName().
     * @note The value of KPluginMetaData::dependencies() is not used here, dependencies must be
     * resolved manually.
     *
     * @return The plugin root object or @c nullptr if it could not be loaded
     * @see QPluginLoader::instance(), KPluginFactory::loadFactory, KPluginFactory::instantiatePlugin
     * @deprecated Since 5.86, use @ref KPluginFactory::loadFactory or @ref KPluginFactory::instantiatePlugin when using
     * KPluginFactory. Otherwise use QPluginLoader::instance() instead.
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "See API docs")
    QObject *instantiate() const;
#endif

    /**
     * @return the user visible name of the plugin.
     */
    QString name() const;

    /**
     * @return a short description of the plugin.
     */
    QString description() const;

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 87)
    /**
     * @return additional information about this plugin (e.g. for use in an "about plugin" dialog)
     *
     * @since 5.18
     * @deprecated Since 5.87, deprecate for lack of usage. Use a meaningful custom key in the json metadata instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 87, "Deprecate for lack of usage, use a meaningful custom key in the json metadata instead")
    QString extraInformation() const;
#endif

    /**
     * @return the author(s) of this plugin.
     */
    QList<KAboutPerson> authors() const;

    /**
     * @return the translator(s) of this plugin.
     *
     * @since 5.18
     */
    QList<KAboutPerson> translators() const;

    /**
     * @return a list of people that contributed to this plugin (other than the authors and translators).
     *
     * @since 5.18
     */
    QList<KAboutPerson> otherContributors() const;

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
     * @return the text of the license, equivalent to KAboutLicense::byKeyword(license()).text()
     * @since 5.73
     */
    QString licenseText() const;

    /**
     * @return a short copyright statement
     *
     * @since 5.18
     */
    QString copyrightText() const;

    /**
     * @return the internal name of the plugin
     * If the Id property is not set in the metadata, this will return the
     * plugin file name without the file extension.
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
     * @return the website where people can report a bug found in this plugin
     * @since 5.99
     */
    QString bugReportUrl() const;

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 79)
    /**
     * @return a list of plugins that this plugin depends on so that it can function properly
     * @see KJsonPluginInfo::pluginId()
     * @deprecated Since 5.79, plugin dependencies are deprecated and will be removed in KF6
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 79, "Plugin dependencies are deprecated and will be removed in KF6")
    QStringList dependencies() const;
#endif

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 89)
    /**
     * Returns the service types that this plugin implements.
     *
     * This is mostly for historical / compatibility purposes.
     * As a general rule, instead of opening many plugins to then filter by servicetype,
     * put all plugins of the same type in a subdirectory, that you can pass to findPlugins directly.
     * No point in opening 20 plugins to pick out only 3, when the filesystem can do that filtering for you.
     *
     * @note Unlike KService this does not contain the MIME types. To get the handled MIME types
     * use the KPluginMetaData::mimeTypes() function.
     * @return a list of service types this plugin implements (e.g. "Plasma/DataEngine")
     * @dprecated Since 5.89, use dedicated plugin namespaces instead to filter plugins of a specific type
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 89, "See API docs")
    QStringList serviceTypes() const;
#endif

    /**
     * @return a list of MIME types this plugin can handle (e.g. "application/pdf", "image/png", etc.)
     * @since 5.16
     */
    QStringList mimeTypes() const;

    /**
     * @return true if this plugin can handle the given mimetype
     * This is more accurate than mimeTypes().contains(mimeType) because it also
     * takes MIME type inheritance into account.
     * @since 5.66
     */
    bool supportsMimeType(const QString &mimeType) const;

    /**
     * @return A string list of formfactors this plugin is useful for, e.g. desktop, tablet,
     * handset, mediacenter, etc.
     * The keys for this are not formally defined.
     *
     * @since 5.12
     */
    QStringList formFactors() const;

    /**
     * @return whether the plugin should be enabled by default.
     * This is only a recommendation, applications can ignore this value if they want to.
     */
    bool isEnabledByDefault() const;

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 104)
    /**
     * @return the initial preference of the plugin.
     * This is the preference to associate with this plugin initially (before
     * the user has had any chance to define preferences for it).
     * Higher values indicate stronger preference.
     * @since 5.67
     * @deprecated Since 5.104, this feature is only used in KParts, read the key manually if needed
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 104, "This feature is only used in KParts, read the key manually if needed")
    int initialPreference() const;
#endif

    /**
     * Returns @c true if the plugin is enabled in @p config, otherwise returns isEnabledByDefault().
     * This can be used in conjunctionwith KPluginWidget/KPluginSelector.
     *
     * The @p config param should be a KConfigGroup object, because KCoreAddons can not depend
     * on KConfig directly, this parameter is a template.
     * @param config KConfigGroup where the enabled state is stored
     * @since 5.89
     */
    template<typename T>
    bool isEnabled(const T &config) const
    {
        Q_ASSERT(config.isValid());
        return config.readEntry(pluginId() + QLatin1String("Enabled"), isEnabledByDefault());
    }

    /**
     * @return the value for @p key from the metadata or @p defaultValue if the key does not exist
     * or the value for @p key is not of type string
     *
     * @see KPluginMetaData::rawData() if QString is not the correct type for @p key
     */
    QString value(const QString &key, const QString &defaultValue = QString()) const;

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 88)
    /**
     * Overload to make sure the bool overload is not taken by accident
     * @overload
     * @since 5.88
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 88, "Construct a QString instead of using a char array, otherwise there the bool overload could be chosen by accident")
    QString value(const QString &key, const char *ch) const
    {
        return value(key, QString::fromLatin1(ch));
    }
#else
    QString value(const QString &key, const char *ch) const = delete;
#endif

    /**
     * @overload
     * @since 5.88
     */
    bool value(const QString &key, bool defaultValue) const;

    /**
     * @overload
     * @since 5.88
     */
    int value(const QString &key, int defaultValue) const;

    /** @return the value for @p key from the metadata or @p defaultValue if the key does not exist.
     * If the type of @p key is string, a list containing just that string will be returned.
     * If the type is array, the list will contain one entry for each array member.
     * @overload
     * @since 5.88
     */
    QStringList value(const QString &key, const QStringList &defaultValue) const;

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 88)
    /** @return the value for @p key inside @p jo as a string list. If the type of @p key is string, a list with containing
     * just that string will be returned, if it is an array the list will contain one entry for each array member.
     * If the key cannot be found an empty list will be returned.
     * @deprecated Since 5.88, use @p value(QString, QStringList) on KPluginMetaData instance instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 88, "Use value(QString, QStringList) on KPluginMetaData instance instead")
    static QStringList readStringList(const QJsonObject &jo, const QString &key);
#endif

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 88)
    /**
     * Reads a value from @p jo but unlike QJsonObject::value() it allows different entries for each locale
     * This is done by appending the locale identifier in brackets to the key (e.g. "[de_DE]" or "[es]")
     * When looking for a key "foo" with German (Germany) locale we will first attempt to read "foo[de_DE]",
     * if that does not exist "foo[de]", finally falling back to "foo" if that also doesn't exist.
     * @return the translated value for @p key from @p jo or @p defaultValue if @p key was not found
     * @deprecated Since 5.88, use KJsonUtils::readTranslatedValue instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 88, "Use KJsonUtils::readTranslatedValue instead")
    static QJsonValue readTranslatedValue(const QJsonObject &jo, const QString &key, const QJsonValue &defaultValue = QJsonValue());
#endif

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 88)
    /**
     * @return the translated value of @p key from @p jo as a string or @p defaultValue if @p key was not found
     * or the value for @p key is not of type string
     * @see KPluginMetaData::readTranslatedValue(const QJsonObject &jo, const QString &key)
     * @deprecated Since 5.88, use KJsonUtils::readTranslatedString instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 88, "Use KJsonUtils::readTranslatedString instead")
    static QString readTranslatedString(const QJsonObject &jo, const QString &key, const QString &defaultValue = QString());
#endif

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

    /**
     * @note for loading plugin the plugin independently of it being static/dynamic
     * use @ref KPluginFactory::loadFactory or @ref KPluginFactory::instantiatePlugin.
     * @return true if the instance represents a static plugin
     * @since 5.89
     */
    bool isStaticPlugin() const;

private:
    KCOREADDONS_NO_EXPORT QJsonObject rootObject() const;
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 92)
    KCOREADDONS_NO_EXPORT void loadFromDesktopFile(const QString &file, const QStringList &serviceTypes);
#endif
    KCOREADDONS_NO_EXPORT void loadFromJsonFile(const QString &file);

private:
    KCOREADDONS_NO_EXPORT QVariantList authorsVariant() const;
    KCOREADDONS_NO_EXPORT QVariantList translatorsVariant() const;
    KCOREADDONS_NO_EXPORT QVariantList otherContributorsVariant() const;
    KCOREADDONS_NO_EXPORT QStaticPlugin staticPlugin() const;
    KCOREADDONS_NO_EXPORT QString requestedFileName() const;

    QJsonObject m_metaData;
    QString m_fileName;
    QExplicitlySharedDataPointer<KPluginMetaDataPrivate> d; // for future binary compatible extensions
    friend class KPluginFactory;
};

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
inline size_t qHash(const KPluginMetaData &md, size_t seed)
#else
inline uint qHash(const KPluginMetaData &md, uint seed)
#endif
{
    return qHash(md.pluginId(), seed);
}

Q_DECLARE_METATYPE(KPluginMetaData)

#endif // KPLUGINMETADATA_H
