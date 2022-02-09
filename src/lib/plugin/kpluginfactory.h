/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINFACTORY_H
#define KPLUGINFACTORY_H

#include "kcoreaddons_export.h"
#include "kpluginmetadata.h"

#include <QObject>
#include <QStringList>
#include <QVariant>
#include <kexportplugin.h> // for source compat

#include <memory>
#include <type_traits>

class QWidget;

class KPluginFactoryPrivate;
namespace KParts
{
class Part;
}
class KPluginMetaData;

#define KPluginFactory_iid "org.kde.KPluginFactory"

/**
 * @relates KPluginFactory
 *
 * Declare a KPluginFactory subclass of the given base factory.
 *
 * @param name the name of the KPluginFactory derived class.
 *
 * @param baseFactory the name of the base class (base factory) to use.
 *                    This must be a KPluginFactory subclass with
 *                    a default constructor.
 *
 * Additional parameters may be additional Qt properties, such as
 * Q_PLUGIN_METADATA.
 *
 * @note The base factory must be a subclass of KPluginFactory.
 * While this macro is largely an implementation detail, factories
 * that have a different create() interface can be declared through
 * this macro. Normal use through other K_PLUGIN_FACTORY macros
 * uses KPluginFactory as a base.
 *
 * @note This macro is usually only an implementation detail
 * for K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY or
 * K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY_JSON .
 *
 * @since 5.80
 *
 */
#define K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY_SKEL(name, baseFactory, ...)                                                                             \
    class name : public baseFactory                                                                                                                            \
    {                                                                                                                                                          \
        Q_OBJECT                                                                                                                                               \
        Q_INTERFACES(KPluginFactory)                                                                                                                           \
        __VA_ARGS__                                                                                                                                            \
    public:                                                                                                                                                    \
        explicit name();                                                                                                                                       \
        ~name();                                                                                                                                               \
    };

#define K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY_JSON(name, baseFactory, json)                                                                            \
    K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY_SKEL(name, baseFactory, Q_PLUGIN_METADATA(IID KPluginFactory_iid FILE json))

#define K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY(name, baseFactory)                                                                                       \
    K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY_SKEL(name, baseFactory, Q_PLUGIN_METADATA(IID KPluginFactory_iid))

#define K_PLUGIN_FACTORY_DEFINITION_WITH_BASEFACTORY(name, baseFactory, pluginRegistrations)                                                                   \
    name::name(){pluginRegistrations} name::~name()                                                                                                            \
    {                                                                                                                                                          \
    }

#define K_PLUGIN_FACTORY_WITH_BASEFACTORY(name, baseFactory, pluginRegistrations)                                                                              \
    K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY(name, baseFactory)                                                                                           \
    K_PLUGIN_FACTORY_DEFINITION_WITH_BASEFACTORY(name, baseFactory, pluginRegistrations)

#define K_PLUGIN_FACTORY_WITH_BASEFACTORY_JSON(name, baseFactory, jsonFile, pluginRegistrations)                                                               \
    K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY_JSON(name, baseFactory, jsonFile)                                                                            \
    K_PLUGIN_FACTORY_DEFINITION_WITH_BASEFACTORY(name, baseFactory, pluginRegistrations)

/**
 * @relates KPluginFactory
 *
 * Create a KPluginFactory subclass and export it as the root plugin object.
 *
 * @param name the name of the KPluginFactory derived class.
 *
 * @param pluginRegistrations code to be inserted into the constructor of the
 * class. Usually a series of registerPlugin() calls.
 *
 * @note K_PLUGIN_FACTORY declares the subclass including a Q_OBJECT macro.
 * So you need to make sure to have Qt's moc run also for the source file
 * where you use the macro. E.g. in projects using CMake and it's automoc feature,
 * as usual you need to have a line
 * @code
 * #include <myplugin.moc>
 * @endcode
 * in the same source file when that one has the name "myplugin.cpp".
 *
 * Example:
 * @code
 * #include <KPluginFactory>
 * #include <plugininterface.h>
 *
 * class MyPlugin : public PluginInterface
 * {
 * public:
 *     MyPlugin(QObject *parent, const QVariantList &args)
 *         : PluginInterface(parent)
 *     {}
 * };
 *
 * K_PLUGIN_FACTORY(MyPluginFactory,
 *                  registerPlugin<MyPlugin>();
 *                 )
 *
 * #include <myplugin.moc>
 * @endcode
 *
 * If you want to compile a .json file into the plugin, use K_PLUGIN_FACTORY_WITH_JSON.
 *
 * @see K_PLUGIN_FACTORY_WITH_JSON
 * @see K_PLUGIN_FACTORY_DECLARATION
 * @see K_PLUGIN_FACTORY_DEFINITION
 */
#define K_PLUGIN_FACTORY(name, pluginRegistrations) K_PLUGIN_FACTORY_WITH_BASEFACTORY(name, KPluginFactory, pluginRegistrations)

/**
 * @relates KPluginFactory
 *
 * Create a KPluginFactory subclass and export it as the root plugin object with
 * JSON metadata.
 *
 * This macro does the same as K_PLUGIN_FACTORY, but adds a JSON file as plugin
 * metadata.  See Q_PLUGIN_METADATA() for more information.
 *
 * @param name the name of the KPluginFactory derived class.
 *
 * @param pluginRegistrations code to be inserted into the constructor of the
 * class. Usually a series of registerPlugin() calls.
 *
 * @param jsonFile name of the JSON file to be compiled into the plugin as metadata
 *
 * @note K_PLUGIN_FACTORY_WITH_JSON declares the subclass including a Q_OBJECT macro.
 * So you need to make sure to have Qt's moc run also for the source file
 * where you use the macro. E.g. in projects using CMake and its automoc feature,
 * as usual you need to have a line
 * @code
 * #include <myplugin.moc>
 * @endcode
 * in the same source file when that one has the name "myplugin.cpp".
 *
 * Example (KF >= 5.77):
 * @code
 * #include <KPluginFactory>
 * #include <plugininterface.h>
 *
 * class MyPlugin : public PluginInterface
 * {
 * public:
 *     MyPlugin(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
 *         : PluginInterface(parent)
 *     {}
 * };
 *
 * K_PLUGIN_FACTORY_WITH_JSON(MyPluginFactory,
 *                  "metadata.json",
 *                  registerPlugin<MyPlugin>();
 *                 )
 *
 * #include <myplugin.moc>
 * @endcode
 *
 * Example (backward-compatible with KF < 5.77):
 * @code
 * #include <KPluginFactory>
 * #include <plugininterface.h>
 *
 * class MyPlugin : public PluginInterface
 * {
 * public:
 *     MyPlugin(QObject *parent, const QVariantList &args)
 *         : PluginInterface(parent)
 *     {}
 * };
 *
 * K_PLUGIN_FACTORY_WITH_JSON(MyPluginFactory,
 *                  "metadata.json",
 *                  registerPlugin<MyPlugin>();
 *                 )
 *
 * #include <myplugin.moc>
 * @endcode
 *
 * @see K_PLUGIN_FACTORY
 * @see K_PLUGIN_FACTORY_DECLARATION
 * @see K_PLUGIN_FACTORY_DEFINITION
 *
 * @since 5.0
 */
#define K_PLUGIN_FACTORY_WITH_JSON(name, jsonFile, pluginRegistrations)                                                                                        \
    K_PLUGIN_FACTORY_WITH_BASEFACTORY_JSON(name, KPluginFactory, jsonFile, pluginRegistrations)

/**
 * @relates KPluginFactory
 *
 * Create a KPluginFactory subclass and export it as the root plugin object with
 * JSON metadata.
 *
 * This macro does the same as K_PLUGIN_FACTORY_WITH_JSON, but you only have to pass the class name and the json file.
 * The factory name and registerPlugin call are deduced from the class name.
 *
 * @code
 * #include <myplugin.moc>
 * @endcode
 * in the same source file when that one has the name "myplugin.cpp".
 *
 * Example (KF >= 5.77):
 * @code
 * #include <KPluginFactory>
 * #include <plugininterface.h>
 *
 * class MyPlugin : public PluginInterface
 * {
 * public:
 *     MyPlugin(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
 *         : PluginInterface(parent)
 *     {}
 * };
 *
 * K_PLUGIN_CLASS_WITH_JSON(MyPlugin, "metadata.json")
 *
 * #include <myplugin.moc>
 * @endcode
 *
 * Example (backward-compatible with KF < 5.77):
 * @code
 * #include <KPluginFactory>
 * #include <plugininterface.h>
 *
 * class MyPlugin : public PluginInterface
 * {
 * public:
 *     MyPlugin(QObject *parent, const QVariantList &args)
 *         : PluginInterface(parent)
 *     {}
 * };
 *
 * K_PLUGIN_CLASS_WITH_JSON(MyPlugin, "metadata.json")
 *
 * #include <myplugin.moc>
 * @endcode
 *
 * @see K_PLUGIN_FACTORY_WITH_JSON
 *
 * @since 5.44
 */
#ifdef KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME
#define K_PLUGIN_CLASS_WITH_JSON(classname, jsonFile)                                                                                                          \
    K_PLUGIN_FACTORY_WITH_JSON(KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME, jsonFile, registerPlugin<classname>();)
#else
#define K_PLUGIN_CLASS_WITH_JSON(classname, jsonFile) K_PLUGIN_FACTORY_WITH_JSON(classname##Factory, jsonFile, registerPlugin<classname>();)
#endif

/**
 * @relates KPluginFactory
 *
 * Creates a KPluginFactory subclass and exports it as the root plugin object.
 * Unlike @ref K_PLUGIN_CLASS_WITH_JSON, this macro does not require json meta data.
 *
 * This macro does the same as K_PLUGIN_FACTORY, but you only have to pass the class name.
 * The factory name and registerPlugin call are deduced from the class name.
 * This is also useful if you want to use static plugins, see the kcoreaddons_add_plugin CMake method.
 * @since 5.90
 */
#ifdef KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME
#define K_PLUGIN_CLASS(classname) K_PLUGIN_FACTORY(KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME, registerPlugin<classname>();)
#else
#define K_PLUGIN_CLASS(classname) K_PLUGIN_FACTORY(classname##Factory, registerPlugin<classname>();)
#endif

/**
 * @relates KPluginFactory
 *
 * K_PLUGIN_FACTORY_DECLARATION declares the KPluginFactory subclass. This macro
 * can be used in a header file.
 *
 * @param name the name of the KPluginFactory derived class.
 *
 * @see K_PLUGIN_FACTORY
 * @see K_PLUGIN_FACTORY_DEFINITION
 */
#define K_PLUGIN_FACTORY_DECLARATION(name) K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY(name, KPluginFactory)

/**
 * @relates KPluginFactory
 * K_PLUGIN_FACTORY_DEFINITION defines the KPluginFactory subclass. This macro
 * can <b>not</b> be used in a header file.
 *
 * @param name the name of the KPluginFactory derived class.
 *
 * @param pluginRegistrations code to be inserted into the constructor of the
 * class. Usually a series of registerPlugin() calls.
 *
 * @see K_PLUGIN_FACTORY
 * @see K_PLUGIN_FACTORY_DECLARATION
 */
#define K_PLUGIN_FACTORY_DEFINITION(name, pluginRegistrations) K_PLUGIN_FACTORY_DEFINITION_WITH_BASEFACTORY(name, KPluginFactory, pluginRegistrations)

/**
 * @class KPluginFactory kpluginfactory.h <KPluginFactory>
 *
 * KPluginFactory provides a convenient way to provide factory-style plugins.
 * Qt plugins provide a singleton object, but a common pattern is for plugins
 * to generate as many objects of a particular type as the application requires.
 * By using KPluginFactory, you can avoid implementing the factory pattern
 * yourself.
 *
 * KPluginFactory also allows plugins to provide multiple different object
 * types, indexed by keywords.
 *
 * The objects created by KPluginFactory must inherit QObject, and must have a
 * standard constructor pattern:
 * @li if the object is a KPart::Part, it must be of the form
 * @code
 * T(QWidget *parentWidget, QObject *parent, const QVariantList &args)
 * @endcode
 * or, since KF 5.77,
 * @code
 * T(QWidget *parentWidget, QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
 * @endcode
 * @li if it is a QWidget, it must be of the form
 * @code
 * T(QWidget *parent, const QVariantList &args)
 * @endcode
 * or, since KF 5.77,
 * @code
 * T(QWidget *parent, const KPluginMetaData &metaData, const QVariantList &args)
 * @endcode
 * @li otherwise it must be of the form
 * @code
 * T(QObject *parent, const QVariantList &args)
 * @endcode
 * or, since KF 5.77,
 * @code
 * T(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
 * @endcode
 *
 * You should typically use either K_PLUGIN_FACTORY() or
 * K_PLUGIN_FACTORY_WITH_JSON() in your plugin code to create the factory.  The
 * typical pattern is
 *
 * @code
 * #include <KPluginFactory>
 * #include <plugininterface.h>
 *
 * class MyPlugin : public PluginInterface
 * {
 * public:
 *     MyPlugin(QObject *parent, const QVariantList &args)
 *         : PluginInterface(parent)
 *     {}
 * };
 *
 * K_PLUGIN_FACTORY(MyPluginFactory,
 *                  registerPlugin<MyPlugin>();
 *                 )
 * #include <myplugin.moc>
 * @endcode
 *
 * If you want to write a custom KPluginFactory not using the standard macro(s)
 * you can reimplement the
 * create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
 * method.
 *
 * Example:
 * @code
 * class SomeScriptLanguageFactory : public KPluginFactory
 * {
 *     Q_OBJECT
 * public:
 *     SomeScriptLanguageFactory()
 *     {}
 *
 * protected:
 *     virtual QObject *create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
 *     {
 *         const QString identifier = QLatin1String(iface) + QLatin1Char('_') + keyword;
 *         // load scripting language module from the information in identifier
 *         // and return it:
 *         return object;
 *     }
 * };
 * @endcode
 *
 * To use such a custom KPluginFactory, use the K_PLUGIN_FACTORY_DECLARATION_WITH_BASEFACTORY
 * and K_PLUGIN_FACTORY_DEFINITION_WITH_BASEFACTORY macros, passing in the
 * name of the custom subclass as @p baseFactory .
 *
 * If you want to load a library use KPluginLoader.
 * The application that wants to instantiate plugin classes can do the following:
 * @code
 * KPluginFactory *factory = KPluginLoader("libraryname").factory();
 * if (factory) {
 *     PluginInterface *p1 = factory->create<PluginInterface>(parent);
 *     OtherInterface *p2  = factory->create<OtherInterface>(parent);
 *     NextInterface *p3   = factory->create<NextInterface>("keyword1", parent);
 *     NextInterface *p3   = factory->create<NextInterface>("keyword2", parent);
 * }
 * @endcode
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @author Bernhard Loos <nhuh.put@web.de>
 */
class KCOREADDONS_EXPORT KPluginFactory : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KPluginFactory)
public:
    /**
     * This constructor creates a factory for a plugin.
     */
    explicit KPluginFactory();

    /**
     * This destroys the PluginFactory.
     */
    ~KPluginFactory() override;

    /// @since 5.86
    enum ResultErrorReason {
        NO_PLUGIN_ERROR = 0,
        INVALID_PLUGIN,
        INVALID_FACTORY,
        INVALID_KPLUGINFACTORY_INSTANTIATION,
    };
    /**
     * Holds the result of a plugin load operation, i.e. the loaded plugin on success or information about the error on failure
     * @since 5.86
     */
    template<typename T>
    class Result
    {
    public:
        T *plugin = nullptr;
        /// translated, user-visible error string
        QString errorString;
        /// untranslated error text
        QString errorText;
        ResultErrorReason errorReason = NO_PLUGIN_ERROR;
        explicit operator bool() const
        {
            return plugin != nullptr;
        }
    };

    /**
     * Attempts to load the KPluginFactory from the given metadata.
     * The errors will be logged using the `kf.coreaddons` debug category.
     * @param data KPluginMetaData from which the plugin should be loaded
     * @return Result object which contains the plugin instance and potentially error information
     * @since 5.86
     */
    static Result<KPluginFactory> loadFactory(const KPluginMetaData &data);

    /**
     * Attempts to load the KPluginFactory and create a @p T instance from the given metadata
     * @code
        KPluginFactory::Result<MyClass> pluginResult = KPluginFactory::instantiatePlugin<MyClass>(metaData, parent, args);
        if (pluginResult) {
            // The plugin is valid and can be accessed
        } else {
            // The pluginResult object contains information about the error
        }
     * @endcode
     * If there is no extra error handling needed the plugin can be directly accessed and checked if it is a nullptr
     * @code
        if (auto plugin = KPluginFactory::instantiatePlugin<MyClass>(metaData, parent, args).plugin) {
            // The plugin is valid and can be accessed
        }
     * @endcode
     * @param data KPluginMetaData from which the plugin should be loaded
     * @return Result object which contains the plugin instance and potentially error information
     * @since 5.86
     */
    template<typename T>
    static Result<T> instantiatePlugin(const KPluginMetaData &data, QObject *parent = nullptr, const QVariantList &args = {})
    {
        Result<T> result;
        KPluginFactory::Result<KPluginFactory> factoryResult = loadFactory(data);
        if (!factoryResult.plugin) {
            result.errorString = factoryResult.errorString;
            result.errorText = factoryResult.errorText;
            result.errorReason = factoryResult.errorReason;
            return result;
        }
        T *instance = factoryResult.plugin->create<T>(parent, args);
        if (!instance) {
            const QLatin1String className(T::staticMetaObject.className());
            result.errorString = tr("KPluginFactory could not create a %1 instance from %2").arg(className, data.fileName());
            result.errorText = QStringLiteral("KPluginFactory could not create a %1 instance from %2").arg(className, data.fileName());
            result.errorReason = INVALID_KPLUGINFACTORY_INSTANTIATION;
            logFailedInstantiationMessage(T::staticMetaObject.className(), data);
        } else {
            result.plugin = instance;
        }
        return result;
    }

    /**
     * Use this method to create an object. It will try to create an object which inherits
     * @p T. If it has multiple choices it's not defined which object will be returned, so be careful
     * to request a unique interface or use keywords.
     *
     * @tparam T the interface for which an object should be created. The object will inherit @p T.
     * @param parent the parent of the object. If @p parent is a widget type, it will also passed
     *               to the parentWidget argument of the CreateInstanceFunction for the object.
     * @param args additional arguments which will be passed to the object.
     * @returns pointer to the created object is returned, or @c nullptr if an error occurred.
     */
    template<typename T>
    T *create(QObject *parent = nullptr, const QVariantList &args = QVariantList());

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 89)
    /**
     * Use this method to create an object. It will try to create an object which inherits
     * @p T and was registered with @p keyword.
     *
     * @tparam T the interface for which an object should be created. The object will inherit @p T.
     * @param keyword the keyword of the object.
     * @param parent the parent of the object. If @p parent is a widget type, it will also passed
     *               to the parentWidget argument of the CreateInstanceFunction for the object.
     * @param args additional arguments which will be passed to the object.
     * @returns pointer to the created object is returned, or @c nullptr if an error occurred.
     * @deprecated Since 5.89, use overload without keyword instead
     */
    template<typename T>
    KCOREADDONS_DEPRECATED_VERSION(5, 89, "Use overload without keyword instead")
    T *create(const QString &keyword, QObject *parent = nullptr, const QVariantList &args = QVariantList());
#endif

    /**
     * Use this method to create an object. It will try to create an object which inherits @p T
     * This overload has an additional @p parentWidget argument, which is used by some plugins (e.g. Parts).

     * @tparam T the interface for which an object should be created. The object will inherit @p T.
     * @param parentWidget an additional parent widget.
     * @param parent the parent of the object. If @p parent is a widget type, it will also passed
     *               to the parentWidget argument of the CreateInstanceFunction for the object.
     * @param args additional arguments which will be passed to the object. Since 5.93 this has a default arg.
     * @returns pointer to the created object is returned, or @c nullptr if an error occurred.
     */
    template<typename T>
    T *create(QWidget *parentWidget, QObject *parent, const QVariantList &args = {});

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 89)
    /**
     * @overload
     * @param keyword the keyword of the object.
     * @deprecated Since 5.89, use overload without keyword instead
     */
    template<typename T>
    KCOREADDONS_DEPRECATED_VERSION(5, 89, "Use overload without keyword instead")
    T *create(QWidget *parentWidget, QObject *parent, const QString &keyword, const QVariantList &args = QVariantList());
#endif

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(4, 0)
    /**
     * @deprecated since 4.0 use create<T>(QObject *parent, const QVariantList &args)
     */
    template<typename T>
    KCOREADDONS_DEPRECATED_VERSION(4, 0, "Use KPluginFactory::create<T>(QObject *parent, const QVariantList &args)")
    T *create(QObject *parent, const QStringList &args)
    {
        return create<T>(parent, stringListToVariantList(args));
    }

    /**
     * @deprecated since 4.0 use create<T>(QObject *parent, const QVariantList &args)
     */
    KCOREADDONS_DEPRECATED_VERSION(4, 0, "Use KPluginFactory::create<T>(QObject *parent, const QVariantList &args)")
    QObject *create(QObject *parent = nullptr, const char *classname = "QObject", const QStringList &args = QStringList())
    {
        return create(classname, nullptr, parent, stringListToVariantList(args), QString());
    }
#endif

    /**
     * @returns the metadata of the plugin
     *
     * @since 5.77
     */
    KPluginMetaData metaData() const;

    /**
     * Set the metadata about the plugin this factory generates.
     *
     * @param metaData  the metadata about the plugin
     *
     * @since 5.77
     */
    void setMetaData(const KPluginMetaData &metaData);

    /**
     * @internal
     * Converts a QStringList to a QVariantList
     */
    static QVariantList stringListToVariantList(const QStringList &list);

    /**
     * @internal
     * Converts a QVariantList of strings to a QStringList
     */
    static QStringList variantListToStringList(const QVariantList &list);

Q_SIGNALS:
    void objectCreated(QObject *object);

protected:
    /**
     * Function pointer type to a function that instantiates a plugin.
     */
    typedef QObject *(*CreateInstanceFunction)(QWidget *, QObject *, const QVariantList &);

    /**
     * This is used to detect the arguments need for the constructor of metadata-less plugin classes.
     * You can inherit it, if you want to add new classes and still keep support for the old ones.
     */
    template<class impl>
    struct InheritanceChecker {
        /// property to control the availability of the registerPlugin overload taking default values
        static constexpr bool enabled = std::is_constructible<impl, QWidget *, QObject *, const QVariantList &>::value
            || std::is_constructible<impl, QWidget *, const QVariantList &>::value || std::is_constructible<impl, QObject *, const QVariantList &>::value;

        CreateInstanceFunction createInstanceFunction(KParts::Part *)
        {
            return &createPartInstance<impl>;
        }
        CreateInstanceFunction createInstanceFunction(QWidget *)
        {
            return &createInstance<impl, QWidget>;
        }
        CreateInstanceFunction createInstanceFunction(...)
        {
            return &createInstance<impl, QObject>;
        }
    };

    /**
     * Function pointer type to a function that instantiates a plugin, also taking a plugin metadata argument.
     * @since 5.77
     */
    using CreateInstanceWithMetaDataFunction = QObject *(*)(QWidget *, QObject *, const KPluginMetaData &, const QVariantList &);

    /**
     * This is used to detect the arguments need for the constructor of metadata-taking plugin classes.
     * You can inherit it, if you want to add new classes and still keep support for the old ones.
     */
    template<class impl>
    struct InheritanceWithMetaDataChecker {
        /// property to control the availability of the registerPlugin overload taking default values
        static constexpr bool enabled = std::is_constructible<impl, QWidget *, QObject *, const KPluginMetaData &, const QVariantList &>::value
            || std::is_constructible<impl, QWidget *, const KPluginMetaData &, const QVariantList &>::value
            || std::is_constructible<impl, QObject *, const KPluginMetaData &, const QVariantList &>::value;

        CreateInstanceWithMetaDataFunction createInstanceFunction(KParts::Part *)
        {
            return &createPartWithMetaDataInstance<impl>;
        }
        CreateInstanceWithMetaDataFunction createInstanceFunction(QWidget *)
        {
            return &createWithMetaDataInstance<impl, QWidget>;
        }
        CreateInstanceWithMetaDataFunction createInstanceFunction(...)
        {
            return &createWithMetaDataInstance<impl, QObject>;
        }
    };

    explicit KPluginFactory(KPluginFactoryPrivate &dd);

    // Use std::enable_if_t once C++14 can be relied on
    template<bool B, class T = void>
    using enable_if_t = typename std::enable_if<B, T>::type;

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 89)
    /**
     * Registers a metadata-less plugin with the factory. Call this function from the constructor of the
     * KPluginFactory subclass to make the create function able to instantiate the plugin when asked
     * for an interface the plugin implements.
     *
     * You can register as many plugin classes as you want as long as either the plugin interface or
     * the @p keyword makes it unique. E.g. it is possible to register a KCModule and a
     * KParts::Part without having to specify keywords since their interfaces differ.
     *
     * @tparam T the name of the plugin class
     *
     * @param keyword an optional keyword as unique identifier for the plugin. This allows you to
     * put more than one plugin with the same interface into the same library using the same
     * factory. X-KDE-PluginKeyword is a convenient way to specify the keyword in a desktop file.
     *
     * @param instanceFunction A function pointer to a function that creates an instance of the
     * plugin.
     * @deprecated Since 5.89, providing a custom CreateInstanceFunction is deprecated. Use registerPlugin<T>() instead
     */
    template<class T>
    KCOREADDONS_DEPRECATED_VERSION_BELATED(5, 89, 5, 95, "Use registerPlugin(CreateInstanceWithMetaDataFunction) instead")
    void registerPlugin(const QString &keyword, CreateInstanceFunction instanceFunction)
    {
        registerPlugin(keyword, &T::staticMetaObject, instanceFunction);
    }
#endif

    /**
     * Overload for registerPlugin<T>(const QString &keyword, CreateInstanceFunction instanceFunction)
     *
     * Uses a default instance creation function depending on the type of interface. If the
     * interface inherits from
     * @li @c KParts::Part the function will call
     * @code
     * new T(QWidget *parentWidget, QObject *parent, const QVariantList &args)
     * @endcode
     * @li @c QWidget the function will call
     * @code
     * new T(QWidget *parent, const QVariantList &args)
     * @endcode
     * @li else the function will call
     * @code
     * new T(QObject *parent, const QVariantList &args)
     * @endcode
     *
     * If those constructor methods are not callable this overload is not available.
     */
    template<class T, enable_if_t<InheritanceChecker<T>::enabled, int> = 0>
    void registerPlugin()
    {
        CreateInstanceFunction instanceFunction = InheritanceChecker<T>().createInstanceFunction(static_cast<T *>(nullptr));
        registerPlugin(QString(), &T::staticMetaObject, instanceFunction);
    }

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 89)
    /**
     * @overload
     * @deprecated Since 5.89, use overload without keyword instead
     */
    template<class T, enable_if_t<InheritanceChecker<T>::enabled, int> = 0>
    KCOREADDONS_DEPRECATED_VERSION(5, 89, "Use overload without keyword instead")
    void registerPlugin(const QString &keyword)
    {
        CreateInstanceFunction instanceFunction = InheritanceChecker<T>().createInstanceFunction(static_cast<T *>(nullptr));
        registerPlugin<T>(keyword, instanceFunction);
    }
#endif

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 89)
    /**
     * Registers a metadata-taking plugin with the factory. Call this function from the constructor of the
     * KPluginFactory subclass to make the create function able to instantiate the plugin when asked
     * for an interface the plugin implements.
     *
     * You can register as many plugin classes as you want as long as either the plugin interface or
     * the @p keyword makes it unique. E.g. it is possible to register a KCModule and a
     * KParts::Part without having to specify keywords since their interfaces differ.
     *
     * @tparam T the name of the plugin class
     *
     * @param keyword An optional keyword as unique identifier for the plugin. This allows you to
     * put more than one plugin with the same interface into the same library using the same
     * factory. X-KDE-PluginKeyword is a convenient way to specify the keyword in a desktop file.
     *
     * @param instanceFunction A function pointer to a function that creates an instance of the
     * plugin.
     * @deprecated Since 5.89, providing a custom CreateInstanceWithMetaDataFunction is deprecated. Use registerPlugin<T>() instead
     */
    template<class T>
    KCOREADDONS_DEPRECATED_VERSION(5, 89, "Providing a custom CreateInstanceWithMetaDataFunction is deprecated. Use registerPlugin<T>() instead")
    void registerPlugin(const QString &keyword, CreateInstanceWithMetaDataFunction instanceFunction)
    {
        registerPlugin(keyword, &T::staticMetaObject, instanceFunction);
    }
#endif

    /**
     * Uses a default instance creation function depending on the type of interface. If the
     * interface inherits from
     * @li @c KParts::Part the function will call
     * @code
     * new T(QWidget *parentWidget, QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
     * @endcode
     * @li @c QWidget the function will call
     * @code
     * new T(QWidget *parent, const KPluginMetaData &metaData, const QVariantList &args)
     * @endcode
     * @li else the function will call
     * @code
     * new T(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
     * @endcode
     *
     * If those constructor methods are not callable this overload is not available.
     */
    template<class T, enable_if_t<InheritanceWithMetaDataChecker<T>::enabled, int> = 0>
    void registerPlugin()
    {
        CreateInstanceWithMetaDataFunction instanceFunction = InheritanceWithMetaDataChecker<T>().createInstanceFunction(static_cast<T *>(nullptr));
        registerPlugin(QString(), &T::staticMetaObject, instanceFunction);
    }

    /**
     * Registers a plugin with the factory. Call this function from the constructor of the
     * KPluginFactory subclass to make the create function able to instantiate the plugin when asked
     * for an interface the plugin implements.
     *
     * @param T the name of the plugin class
     * @param instanceFunction A function pointer to a function that creates an instance of the plugin.
     * @since 5.96
     */
    template<class T>
    void registerPlugin(CreateInstanceWithMetaDataFunction instanceFunction)
    {
        registerPlugin(QString(), &T::staticMetaObject, instanceFunction);
    }

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 89)
    /**
     * @overload
     * @deprecated Since 5.89, use overload without keyword instead
     */
    template<class T, enable_if_t<InheritanceWithMetaDataChecker<T>::enabled, int> = 0>
    KCOREADDONS_DEPRECATED_VERSION(5, 89, "Use overload without keyword instead")
    void registerPlugin(const QString &keyword)
    {
        CreateInstanceWithMetaDataFunction instanceFunction = InheritanceWithMetaDataChecker<T>().createInstanceFunction(static_cast<T *>(nullptr));
        registerPlugin<T>(keyword, instanceFunction);
    }
#endif
    std::unique_ptr<KPluginFactoryPrivate> const d_ptr;

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(4, 0)
    /**
     * @deprecated since 4.0 use create<T>(QObject *parent, const QVariantList &args)
     */
    KCOREADDONS_DEPRECATED_VERSION(4, 0, "Use KPluginFactory::create<T>(QObject *parent, const QVariantList &args)")
    virtual QObject *createObject(QObject *parent, const char *className, const QStringList &args);

    /**
     * @deprecated since 4.0 use create<T>(QWidget *parentWidget, QObject *parent, const QString &keyword, const QVariantList &args)
     */
    KCOREADDONS_DEPRECATED_VERSION(4,
                                   0,
                                   "Use KPluginFactory::create<T>(QWidget *parentWidget, QObject *parent, const QString &keyword, const QVariantList &args)")
    virtual KParts::Part *createPartObject(QWidget *parentWidget, QObject *parent, const char *classname, const QStringList &args);
#endif

    /**
     * This function is called when the factory asked to create an Object.
     *
     * You may reimplement it to provide a very flexible factory. This is especially useful to
     * provide generic factories for plugins implemented using a scripting language.
     *
     * @param iface the staticMetaObject::className() string identifying the plugin interface that
     * was requested. E.g. for KCModule plugins this string will be "KCModule".
     * @param parentWidget only used if the requested plugin is a KPart.
     * @param parent the parent object for the plugin object.
     * @param args a plugin specific list of arbitrary arguments.
     * @param keyword a string that uniquely identifies the plugin. If a KService is used this
     * keyword is read from the X-KDE-PluginKeyword entry in the .desktop file.
     */
    virtual QObject *create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword);

    template<class impl, class ParentType>
    static QObject *createInstance(QWidget *parentWidget, QObject *parent, const QVariantList &args)
    {
        Q_UNUSED(parentWidget)
        ParentType *p = nullptr;
        if (parent) {
            p = qobject_cast<ParentType *>(parent);
            Q_ASSERT(p);
        }
        return new impl(p, args);
    }

    template<class impl>
    static QObject *createPartInstance(QWidget *parentWidget, QObject *parent, const QVariantList &args)
    {
        return new impl(parentWidget, parent, args);
    }

    template<class impl, class ParentType>
    static QObject *createWithMetaDataInstance(QWidget *parentWidget, QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    {
        Q_UNUSED(parentWidget)
        ParentType *p = nullptr;
        if (parent) {
            p = qobject_cast<ParentType *>(parent);
            Q_ASSERT(p);
        }
        return new impl(p, metaData, args);
    }

    template<class impl>
    static QObject *createPartWithMetaDataInstance(QWidget *parentWidget, QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    {
        return new impl(parentWidget, parent, metaData, args);
    }

private:
    void registerPlugin(const QString &keyword, const QMetaObject *metaObject, CreateInstanceFunction instanceFunction);
    void registerPlugin(const QString &keyword, const QMetaObject *metaObject, CreateInstanceWithMetaDataFunction instanceFunction);
    // The logging categories are not part of the public API, consequently this needs to be a private function
    static void logFailedInstantiationMessage(KPluginMetaData data);
    static void logFailedInstantiationMessage(const char *className, KPluginMetaData data);
};

// Deprecation wrapper macro added only for 5.70, while backward typedef added in 4.0
#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 70)
/**
 * Backward compatibility typedef for KPluginFactory
 * @deprecated since 4.0, use KPluginFactory
 */
typedef KPluginFactory KLibFactory;
#endif

template<typename T>
inline T *KPluginFactory::create(QObject *parent, const QVariantList &args)
{
    QObject *o =
        create(T::staticMetaObject.className(), parent && parent->isWidgetType() ? reinterpret_cast<QWidget *>(parent) : nullptr, parent, args, QString());

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 89)
template<typename T>
inline T *KPluginFactory::create(const QString &keyword, QObject *parent, const QVariantList &args)
{
    QObject *o =
        create(T::staticMetaObject.className(), parent && parent->isWidgetType() ? reinterpret_cast<QWidget *>(parent) : nullptr, parent, args, keyword);

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}
#endif

template<typename T>
inline T *KPluginFactory::create(QWidget *parentWidget, QObject *parent, const QVariantList &args)
{
    QObject *o = create(T::staticMetaObject.className(), parentWidget, parent, args, QString());

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 89)
template<typename T>
inline T *KPluginFactory::create(QWidget *parentWidget, QObject *parent, const QString &keyword, const QVariantList &args)
{
    QObject *o = create(T::staticMetaObject.className(), parentWidget, parent, args, keyword);

    T *t = qobject_cast<T *>(o);
    if (!t) {
        delete o;
    }
    return t;
}
#endif

Q_DECLARE_INTERFACE(KPluginFactory, KPluginFactory_iid)

#endif // KPLUGINFACTORY_H
