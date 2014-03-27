/*  This file is part of the KDE project
    Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

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
#ifndef KPLUGINLOADER_H
#define KPLUGINLOADER_H

#include <kexportplugin.h>

#include <QtCore/QPluginLoader>
#include <QtCore/QtPlugin>

class KPluginFactory;
class KService;

class KPluginLoaderPrivate;

/**
 * \class KPluginLoader kpluginloader.h <KPluginLoader>
 *
 * This class behaves largely like QPluginLoader (and, indeed, uses it
 * internally).  It extends its functionality in three ways:
 *
 * - it additionally searches for plugins in the "kf5" subdirectories of
 *   directories in QCoreApplication::libraryPaths() (see findPlugin()); this
 *   corresponds to the ${PLUGIN_INSTALL_DIR} of the KDEInstallDirs CMake module
 *   in extra-cmake-modules
 * - it reads the plugin version, as provided by the K_EXPORT_PLUGIN_VERSION
 *   macro (see pluginVersion())
 * - it provides access to a KPluginFactory instance if the plugin provides one
 *   (see factory())
 *
 * Unlike QPluginLoader, it is not possible to re-use KPluginLoader for more
 * than one plugin (it provides no setFileName method).
 *
 * This class is reentrant, you can load plugins from different threads. You can
 * also have multiple PluginLoaders for one library without negative effects.
 * The object obtained with factory() or the inherited method
 * QPluginLoader::instance() is cached inside the library. If you call factory()
 * or instance() multiple times, you will always get the same object, even from
 * different threads and different KPluginLoader instances.  You can delete this
 * object easily, a new one will be created if factory() or instance() is called
 * afterwards. factory() uses instance() internally.
 *
 * KPluginLoader inherits QPluginLoader::unload(). It is safe to call this
 * method if you loaded a plugin and decide not to use it for some reason. But
 * as soon as you start to use the factory from the plugin, you should stay away
 * from it. It's nearly impossible to keep track of all objects created directly
 * or indirectly from the plugin and all other pointers into plugin code. Using
 * unload() in this case is asking for trouble. If you really need to unload
 * your plugins, you have to take care to convert the clipboard content to text,
 * because the plugin could have registered a custom mime source. You also have
 * to delete the factory of the plugin, otherwise you will create a leak.  The
 * destructor of KPluginLoader doesn't call unload.
 *
 * Sample code:
 * \code
 *  KPluginLoader loader( ...library or kservice... );
 *  KPluginFactory* factory = loader.factory();
 *  if (!factory) {
 *      kWarning() << "Error loading plugin:" << loader.errorString();
 *  } else {
 *      MyInterface* obj = factory->create<MyInterface>();
 *      if (!obj) {
 *          kWarning() << "Error creating object";
 *      }
 *  }
 * \endcode
 *
 * \see KPluginFactory
 *
 * \author Bernhard Loos <nhuh.put@web.de>
 */
class KSERVICE_EXPORT KPluginLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QLibrary::LoadHints loadHints READ loadHints WRITE setLoadHints)
    Q_PROPERTY(QString pluginName READ pluginName)
    Q_PROPERTY(quint32 pluginVersion READ pluginVersion)
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
    explicit KPluginLoader(const QString &plugin, QObject *parent = 0);

    /**
     * Load a plugin from a service.
     *
     * The service must contain a library.  fileName() will be empty if the
     * service did not contain a library or if the library could not be found.
     *
     * \param service The service that provides the plugin.
     * \param parent A parent object.
     */
    explicit KPluginLoader(const KService &service, QObject *parent = 0);

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
     * \returns The factory of the plugin or 0 on error.
     */
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
     */
    QString pluginName() const;

    /**
     * Returns the plugin version.
     *
     * This will load the plugin if it is not already loaded.
     *
     * \returns The version given to K_EXPORT_PLUGIN_VERSION, or (quint32) -1 if
     *          the macro was not used or the plugin could not be loaded.
     */
    quint32 pluginVersion();

    /**
     * Locates a plugin.
     *
     * Searches for a dynamic object file in the locations specified by
     * QCoreApplication::libraryPaths(), and the "kf5" subdirectories of those
     * locations.  Plugins in "kf5" subdirectories will be preferred over ones
     * in the parent directory.
     *
     * This can be useful if you wish to use a plugin that does not conform to
     * the Qt plugin scheme of providing a QObject that declares
     * Q_PLUGIN_METADATA and does not use one of the KPluginFactory macros.
     * In this case, you can find the plugin with this method, and load it with
     * QLibrary.
     *
     * \param name  The name of the plugin (can be a relative path; see above).
     *              This should not include a file extension (like .so or .dll).
     * \returns     The full path of the plugin if it was found, or QString() if
     *              it could not be found.
     *
     * @since 5.0
     */
    static QString findPlugin(const QString &name);

    /**
     * Returns the last error.
     *
     * \returns The description of the last error.
     *
     * \see QPluginLoader::errorString()
     */
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
     */
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
     */
    QObject *instance();

    /**
     * Determines whether the plugin is loaded.
     *
     * \returns  @c True if the plugin is loaded, @c false otherwise.
     *
     * \see QPluginLoader::isLoaded()
     */
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
     */
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
     */
    QLibrary::LoadHints loadHints() const;

    /**
     * Returns the meta data for the plugin.
     *
     * \returns  A JSON object containing the plugin's metadata, if found.
     *
     * \see QPluginLoader::metaData()
     */
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
     */
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
     */
    bool unload();

private:
    Q_DECLARE_PRIVATE(KPluginLoader)
    Q_DISABLE_COPY(KPluginLoader)

    KPluginLoaderPrivate *const d_ptr;
};

#endif
