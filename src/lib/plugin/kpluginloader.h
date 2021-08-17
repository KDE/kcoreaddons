/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef KPLUGINLOADER_H
#define KPLUGINLOADER_H

#include <kcoreaddons_export.h>

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 86)

#include <kexportplugin.h>

#include <QPluginLoader>

#include <functional>
#include <memory>

class KPluginFactory;
class KPluginMetaData;

class KPluginLoaderPrivate;
class KPluginName;

/**
 * \class KPluginLoader kpluginloader.h <KPluginLoader>
 *
 * This class behaves largely like QPluginLoader (and, indeed, uses it
 * internally), but additionally reads the plugin version, as provided by the
 * K_EXPORT_PLUGIN_VERSION macro (see pluginVersion()) and provides access to a
 * KPluginFactory instance if the plugin provides one (see factory())
 *
 * Note that the factory() is a typesafe convenience method that just wraps a
 * qobject_cast on the result of QPluginLoader::instance(). Therefore, if you do
 * not need the plugin version feature, you can (and should) just use
 * QPluginLoader instead.
 *
 * Unlike QPluginLoader, it is not possible to re-use KPluginLoader for more
 * than one plugin (it provides no setFileName method).
 *
 * The same notes and caveats that apply to QPluginLoader also apply to
 * KPluginLoader.
 *
 * Sample code:
 * \code
 *  KPluginLoader loader( ...library or kservice... );
 *  KPluginFactory* factory = loader.factory();
 *  if (!factory) {
 *      qWarning() << "Error loading plugin:" << loader.errorString();
 *  } else {
 *      MyInterface* obj = factory->create<MyInterface>();
 *      if (!obj) {
 *          qWarning() << "Error creating object";
 *      }
 *  }
 * \endcode
 *
 * \see KPluginFactory
 *
 * \author Bernhard Loos <nhuh.put@web.de>
 * @deprecated Since 5.86, the @p findPluginsById, @p findPlugins and @p factory methods have been imported to @ref KPluginMetaData.
 * Check the deprecating messages of the other search related methods for porting instructions.
 * The methods which are exclusively used to work around QTBUG-39642 are
 * deprecated in favour of using QPluginLoader directly. With Qt 5.15 this bug has been fixed.
 */
class KCOREADDONS_EXPORT KPluginLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QLibrary::LoadHints loadHints READ loadHints WRITE setLoadHints)
    Q_PROPERTY(QString pluginName READ pluginName)
#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 84)
    Q_PROPERTY(quint32 pluginVersion READ pluginVersion)
#endif
public:
    /**
     * Load a plugin by name.
     *
     * This should be the name of the plugin object file, without any suffix
     * (like .so or .dll).  Plugin object files should not have a 'lib' prefix.
     *
     * fileName() will be empty if the plugin could not be found.
     *
     * \param plugin The name of the plugin.
     * \param parent A parent object.
     */
    explicit KPluginLoader(const QString &plugin, QObject *parent = nullptr);

    /**
     * Load a plugin by name.
     *
     * This constructor behaves exactly the same as
     * KPluginLoader(const QString&,QObject*).  It allows any class that can be
     * cast to KPluginName (such as KService) to be passed to KPluginLoader.
     *
     * \param name   The name of the plugin.
     * \param parent A parent object.
     */
    explicit KPluginLoader(const KPluginName &name, QObject *parent = nullptr);

    /**
     * Destroys the plugin loader.
     *
     * Unless unload() was called explicitly, the plugin stays in memory until
     * the application terminates.
     */
    ~KPluginLoader();

    /**
     * Returns the factory object of the plugin.
     *
     * This is typically created by one of the KPluginFactory macros.
     * Internally, this uses QPluginLoader::instance(), and the same
     * behaviours apply.
     *
     * Since KF 5.77, the factory will have the metadata set fetched from
     * any JSON metadata that is embedded into the plugin binary.
     *
     * \returns The factory of the plugin or @c nullptr on error.
     * @deprecated Since 5.86, use @ref KPluginMetaData::factory instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use KPluginFactory::loadFactory or KPluginFactory::instantiatePlugin instead")
    KPluginFactory *factory();

    /**
     * Returns the name of this plugin as given to the constructor.
     *
     * If the KService constructor was used, this is the name of the library
     * provided by the service.
     *
     * \returns The plugin name.
     *
     * \see fileName()
     * @deprecated Since 5.86, use @ref KPluginMetaData::pluginId instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use KPluginMetaData::pluginId instead")
    QString pluginName() const;

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 84)
    /**
     * Returns the plugin version.
     *
     * This will load the plugin if it is not already loaded.
     *
     * \returns The version given to K_EXPORT_PLUGIN_VERSION, or (quint32) -1 if
     *          the macro was not used or the plugin could not be loaded.
     * @deprecated Since 5.84, use @ref KPluginMetaData::version() or a versioned namespace instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 84, "Use KPluginMetaData::version or a versioned namespace instead")
    quint32 pluginVersion();
#endif

    /**
     * Locates a plugin.
     *
     * Searches for a dynamic object file in the locations KPluginLoader and
     * QPluginLoader would search (ie: the current directory and
     * QCoreApplication::libraryPaths()).
     *
     * This can be useful if you wish to use a plugin that does not conform to
     * the Qt plugin scheme of providing a QObject that declares
     * Q_PLUGIN_METADATA.  In this case, you can find the plugin with this
     * method, and load it with QLibrary.
     *
     * Note that the path is not necessarily absolute. In particular, if the
     * plugin is found in the current directory, it will be a relative path.
     *
     * \param name  The name of the plugin (can be a relative path; see above).
     *              This should not include a file extension (like .so or .dll).
     * \returns     The path to the plugin if it was found, or QString() if it
     *              could not be found.
     *
     * @since 5.0
     * @deprecated Since 5.86, contruct a QPluginLoader and call fileName() or use KPluginMetaData::findPluginById instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Contruct a QPluginLoader and call fileName() or use KPluginMetaData::findPluginById instead")
    static QString findPlugin(const QString &name);

    /**
     * Returns the last error.
     *
     * \returns The description of the last error.
     *
     * \see QPluginLoader::errorString()
     * @deprecated Since 5.86, use QPluginLoader::errorString() instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use QPluginLoader::errorString() instead")
    QString errorString() const;

    /**
     * Returns the path of the plugin.
     *
     * This will be the full path of the plugin if it was found, and empty if
     * it could not be found.
     *
     * \returns The full path of the plugin, or the null string if it could
     *          not be found.
     *
     * \see QPluginLoader::fileName(), pluginName()
     * @deprecated Since 5.86, use QPluginLoader::fileName() instead, in case of complex query logic consider using @ref KPluginLoader::findPlugins
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use QPluginLoader::fileName() instead, in case of complex query logic consider using KPluginLoader::findPlugins")
    QString fileName() const;

    /**
     * Returns the root object of the plugin.
     *
     * The plugin will be loaded if necessary.  If the plugin used one of the
     * KPluginFactory macros, you should use factory() instead.
     *
     * \returns The plugin's root object.
     *
     * \see QPluginLoader::instance()
     * @deprecated Since 5.86, use QPluginLoader::instance() instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use QPluginLoader::instance() instead")
    QObject *instance();

    /**
     * Determines whether the plugin is loaded.
     *
     * \returns  @c True if the plugin is loaded, @c false otherwise.
     *
     * \see QPluginLoader::isLoaded()
     * @deprecated Since 5.86, use QPluginLoader::isLoaded() instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use QPluginLoader::isLoaded() instead")
    bool isLoaded() const;

    /**
     * Loads the plugin.
     *
     * It is safe to call this multiple times; if the plugin was already loaded,
     * it will just return @c true.
     *
     * Methods that require the plugin to be loaded will load it as necessary
     * anyway, so you do not normally need to call this method.
     *
     * \returns  @c True if the plugin was loaded successfully, @c false
     *           otherwise.
     *
     * \see QPluginLoader::load()
     * @deprecated Since 5.86, use QPluginLoader::load() instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use QPluginLoader::load() instead")
    bool load();

    /**
     * Returns the load hints for the plugin.
     *
     * Determines how load() should work.  See QLibrary::loadHints for more
     * information.
     *
     * \returns  The load hints for the plugin.
     *
     * \see QPluginLoader::loadHints(), setLoadHints()
     * @deprecated Since 5.86, use QPluginLoader::loadHints() instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use QPluginLoader::loadHints() instead")
    QLibrary::LoadHints loadHints() const;

    /**
     * Returns the meta data for the plugin.
     *
     * \returns  A JSON object containing the plugin's metadata, if found.
     *
     * \see QPluginLoader::metaData()
     * @deprecated Since 5.86, use QPluginLoader::metaData() or @ref KPluginMetaData instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use QPluginLoader::metaData() or KPluginMetaData instead")
    QJsonObject metaData() const;

    /**
     * Set the load hints for the plugin.
     *
     * Determines how load() should work.  See QLibrary::loadHints for more
     * information.
     *
     * \param loadHints  The load hints for the plugin.
     *
     * \see QPluginLoader::setLoadHints(), loadHints()
     * @deprecated Since 5.86, use QPluginLoader::setLoadHints instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use QPluginLoader::setLoadHints instead")
    void setLoadHints(QLibrary::LoadHints loadHints);

    /**
     * Attempts to unload the plugin.
     *
     * If other instances of KPluginLoader or QPluginLoader are using the same
     * plugin, this will fail; unloading will only happen when every instance
     * has called unload().
     *
     * \returns  @c True if the plugin was unloaded, @c false otherwise.
     *
     * \see QPluginLoader::unload(), load(), instance(), factory()
     * @deprecated Since 5.86, use QPluginLoader::unload() instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use QPluginLoader::unload() instead")
    bool unload();

    /**
     * Finds and instantiates (by calling QPluginLoader::instance()) all plugins from a given
     * directory. Only plugins which have JSON metadata will be considered. A filter can be passed
     * which determines which of the found plugins should actually be loaded.
     *
     * If you use KConfig you could have a group "Plugins" in your configuration file with the
     * plugin name as the key and true/false as the value to indicate whether the plugin should
     * be loaded. In order to easily load all the enable plugins you could use the following code:
     * @code
     * KConfigGroup pluginGroup = KSharedConfig::openConfig()->group("Plugins");
     * auto filter = [&](const KPluginMetaData &md) {
     *     if (!pluginGroup.hasKey(md.pluginName())) {
     *         return md.isEnabledByDefault();
     *     } else {
     *         return pluginGroup.readEntry(md.pluginName(), false);
     *     }
     * };
     * QList<QObject*> plugins = KPluginLoader::instantiatePlugins("myapp", filter);
     * @endcode
     *
     * @param directory the directory to search for plugins. If a relative path is given for @p directory,
     * all entries of  QCoreApplication::libraryPaths() will be checked with @p directory appended as a
     * subdirectory. If an absolute path is given only that directory will be searched.
     *
     * @param filter a callback function that returns @c true if the found plugin should be loaded
     * and @c false if it should be skipped. If this argument is omitted all plugins will be loaded.
     *
     * @param parent the parent to set for the instantiated plugins, if the
     * plugins were not already loaded.
     *
     * @note If the plugins have been previously loaded (via QPluginLoader,
     * directly or due to this class) without being deleted in the meantime
     * then they are not re-created or re-parented and will be returned using
     * the parent they were originally created with. @sa
     * QPluginLoader::instance().
     *
     * @return a list containing an instantiation of each plugin that met the filter criteria
     *
     * @see KPluginLoader::findPlugins()
     *
     * @since 5.1
     * @deprecated Since 5.86 create instances from the KPluginMetaData::findPlugins result instead. In case
     * you are using KPluginFactory use KPluginMetaData::factory on the returned metadata objects. Otherwise use
     * @ref KPluginMetaData::instantiate or QPluginLoader on the returned metadata objects.
     *
     * @see KPluginFactory::loadFactory, KPluginFactory::instantiatePlugin
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Create instances from the KPluginMetaData::findPlugins result instead")
    static QList<QObject *> instantiatePlugins(const QString &directory,
                                               std::function<bool(const KPluginMetaData &)> filter = std::function<bool(const KPluginMetaData &)>(),
                                               QObject *parent = nullptr);

    /**
     * Find all plugins inside @p directory. Only plugins which have JSON metadata will be considered.
     *
     * @param directory The directory to search for plugins. If a relative path is given for @p directory,
     * all entries of QCoreApplication::libraryPaths() will be checked with @p directory appended as a
     * subdirectory. If an absolute path is given only that directory will be searched.
     *
     * @param filter a callback function that returns @c true if the found plugin should be loaded
     * and @c false if it should be skipped. If this argument is omitted all plugins will be loaded.
     *
     * @return all plugins found in @p directory that fulfil the constraints of @p filter
     *
     * @see KPluginLoader::instantiatePlugins()
     *
     * @since 5.1
     * @deprecated Since 5.86 use @ref KPluginMetaData::findPlugins instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use KPluginMetaData::findPlugins instead")
    static QVector<KPluginMetaData> findPlugins(const QString &directory,
                                                std::function<bool(const KPluginMetaData &)> filter = std::function<bool(const KPluginMetaData &)>());

    /**
     * Find all plugins inside @p directory with a given pluginId. Only plugins which have JSON metadata will be considered.
     *
     * @param directory The directory to search for plugins. If a relative path is given for @p directory,
     * all entries of QCoreApplication::libraryPaths() will be checked with @p directory appended as a
     * subdirectory. If an absolute path is given only that directory will be searched.
     *
     * @param pluginId The Id of the plugin, for example KPluginMetaData.pluginId().
     *
     * @return all plugins found in @p directory with the given pluginId.
     *
     * @see KPluginLoader::instantiatePlugins()
     *
     * @since 5.11
     * @deprecated Since 5.86 use @ref KPluginMetaData::findPluginsById instead
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use KPluginMetaData::findPluginsById instead")
    static QVector<KPluginMetaData> findPluginsById(const QString &directory, const QString &pluginId);

    /**
     * Invokes @p callback for each valid plugin found inside @p directory. This is useful if
     * your application needs to customize the behaviour of KPluginLoader::findPlugins() or
     * KPluginLoader::instantiatePlugins().
     *
     * @note The files found do not necessarily contain JSON metadata and may not be loadable using K/QPluginLoader.
     * The only guarantee made is that they are valid library file names as determined by QLibrary::isLibrary().
     *
     * @param directory The directory to search for plugins. If a relative path is given for @p directory,
     * all entries of QCoreApplication::libraryPaths() will be checked with @p directory appended as a
     * subdirectory. If an absolute path is given only that directory will be searched.
     *
     * @param callback This function will be invoked for each valid plugin that is found. It will receive
     * the absolute path to the plugin as an argument
     *
     * @see KPluginLoader::findPlugins(), KPluginLoader::instantiatePlugins()
     *
     * @since 5.1
     * @deprecated Since 5.86 use @ref KPluginMetaData::findPlugins instead. This will warn about invalid plugin
     * metadata and only return valid results.
     */
    KCOREADDONS_DEPRECATED_VERSION(5, 86, "Use KPluginMetaData::findPlugins instead")
    static void forEachPlugin(const QString &directory, std::function<void(const QString &)> callback = std::function<void(const QString &)>());

private:
    Q_DECLARE_PRIVATE(KPluginLoader)
    Q_DISABLE_COPY(KPluginLoader)

    std::unique_ptr<KPluginLoaderPrivate> const d_ptr;
};

/**
 * Represents the name of a plugin intended for KPluginLoader.
 *
 * This exists only so that classes such as KService can provide a cast
 * operator to allow them to be used as arguments to KPluginLoader.
 * Unless you are implementing such a cast operator, you should never
 * need to use this class directly.
 */
// NOTE: this class is all inline, as it mainly exists for typing reasons
//       (ie: to prevent the issues that would be caused by adding an
//       operator QString() method to KService)
class KCOREADDONS_EXPORT KPluginName
{
public:
    /**
     * Construct a (valid) plugin name from a string.
     *
     * If there was an error and the name could not be determined,
     * fromErrorString() should be used instead to construct an
     * appropriate error message.
     *
     * @param name  The name of the plugin; this should not be empty.
     */
    inline explicit KPluginName(const QString &name);

    /**
     * The name of the plugin.
     *
     * @returns  The string passed to the constructor if isValid() is
     *           @c true, QString() otherwise.
     */
    inline QString name() const;

    /**
     * Whether the name is valid.
     *
     * Note that this only determines how the KPluginName was
     * constructed, not anything about the value of the string.
     *
     * @returns @c true if the KPluginName(const QString&) constructor
     *          was used, @c false if fromErrorString() was used.
     */
    inline bool isValid() const;

    /**
     * The error string if no name could be determined.
     *
     * @returns  The string passed to fromErrorString() if isValid() is
     *           @c false, QString() otherwise.
     */
    inline QString errorString() const;

    /**
     * Construct an invalid plugin name with an error message.
     *
     * When this object is passed to KPluginLoader, @p errorString
     * will be used for KPluginLoader::errorString().
     *
     * @param errorString  The (translated) error string.
     */
    static inline KPluginName fromErrorString(const QString &errorString);

private:
    inline KPluginName(const QString &name, bool isError);

    QString m_name;
    bool m_isError;
};

inline KPluginName::KPluginName(const QString &name)
    : m_name(name)
    , m_isError(false)
{
}
inline KPluginName::KPluginName(const QString &name, bool isError)
    : m_name(name)
    , m_isError(isError)
{
}
inline QString KPluginName::name() const
{
    return m_isError ? QString() : m_name;
}
inline bool KPluginName::isValid() const
{
    return !m_isError;
}
inline QString KPluginName::errorString() const
{
    return m_isError ? m_name : QString();
}
inline KPluginName KPluginName::fromErrorString(const QString &errorString)
{
    return KPluginName(errorString, true);
}

#endif
#endif
