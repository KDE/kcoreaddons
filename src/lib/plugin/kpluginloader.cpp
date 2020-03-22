/*
    * This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kpluginloader.h"

#include "kpluginfactory.h"
#include "kpluginmetadata.h"

#include <QLibrary>
#include <QDir>
#include <QDirIterator>
#include "kcoreaddons_debug.h"
#include <QCoreApplication>
#include <QMutex>

// TODO: Upstream the versioning stuff to Qt
// TODO: Patch for Qt to expose plugin-finding code directly
// TODO: Add a convenience method to KFactory to replace KPluginLoader::factory()
// TODO: (after the above) deprecate this class

class KPluginLoaderPrivate
{
    Q_DECLARE_PUBLIC(KPluginLoader)
protected:
    KPluginLoaderPrivate(const QString &libname)
        : name(libname),
          loader(nullptr),
          pluginVersion(~0U),
          pluginVersionResolved(false)
    {}
    ~KPluginLoaderPrivate()
    {}

    KPluginLoader *q_ptr;
    const QString name;
    QString errorString;
    QPluginLoader *loader;
    quint32 pluginVersion;
    bool pluginVersionResolved;
};

QString KPluginLoader::findPlugin(const QString &name)
{
    // We just defer to Qt; unfortunately, QPluginLoader's searching code is not
    // accessible without creating a QPluginLoader object.

    // Workaround for QTBUG-39642
    static QMutex s_qtWorkaroundMutex;
    QMutexLocker lock(&s_qtWorkaroundMutex);

    QPluginLoader loader(name);
    return loader.fileName();
}

KPluginLoader::KPluginLoader(const QString &plugin, QObject *parent)
    : QObject(parent),
      d_ptr(new KPluginLoaderPrivate(plugin))
{
    d_ptr->q_ptr = this;
    Q_D(KPluginLoader);

    d->loader = new QPluginLoader(plugin, this);
}

KPluginLoader::KPluginLoader(const KPluginName &pluginName, QObject *parent)
    : QObject(parent),
      d_ptr(new KPluginLoaderPrivate(pluginName.name()))
{
    d_ptr->q_ptr = this;
    Q_D(KPluginLoader);

    d->loader = new QPluginLoader(this);

    if (pluginName.isValid()) {
        d->loader->setFileName(pluginName.name());
        if (d->loader->fileName().isEmpty()) {
            qCDebug(KCOREADDONS_DEBUG) << "Failed to load plugin" << pluginName.name() << d->loader->errorString()
                       << "\nPlugin search paths are" << QCoreApplication::libraryPaths()
                       << "\nThe environment variable QT_PLUGIN_PATH might be not correctly set";
        }
    } else {
        d->errorString = pluginName.errorString();
    }
}

KPluginLoader::~KPluginLoader()
{
    delete d_ptr;
}

KPluginFactory *KPluginLoader::factory()
{
    Q_D(KPluginLoader);

    QObject *obj = instance();

    if (!obj) {
        return nullptr;
    }

    KPluginFactory *factory = qobject_cast<KPluginFactory *>(obj);

    if (factory == nullptr) {
        qCDebug(KCOREADDONS_DEBUG) << "Expected a KPluginFactory, got a" << obj->metaObject()->className();
        delete obj;
        d->errorString = tr("The library %1 does not offer a KPluginFactory.").arg(d->name);
    }

    return factory;
}

quint32 KPluginLoader::pluginVersion()
{
    Q_D(const KPluginLoader);

    if (!load()) {
        return qint32(-1);
    }
    return d->pluginVersion;
}

QString KPluginLoader::pluginName() const
{
    Q_D(const KPluginLoader);

    return d->name;
}

QString KPluginLoader::errorString() const
{
    Q_D(const KPluginLoader);

    if (!d->errorString.isEmpty()) {
        return d->errorString;
    }

    return d->loader->errorString();
}

QString KPluginLoader::fileName() const
{
    Q_D(const KPluginLoader);
    return d->loader->fileName();
}

QObject *KPluginLoader::instance()
{
    Q_D(const KPluginLoader);

    if (!load()) {
        return nullptr;
    }

    return d->loader->instance();
}

bool KPluginLoader::isLoaded() const
{
    Q_D(const KPluginLoader);

    return d->loader->isLoaded() && d->pluginVersionResolved;
}

bool KPluginLoader::load()
{
    Q_D(KPluginLoader);

    if (!d->loader->load()) {
        return false;
    }

    if (d->pluginVersionResolved) {
        return true;
    }

    Q_ASSERT(!fileName().isEmpty());
    QLibrary lib(fileName());
    Q_ASSERT(lib.isLoaded()); // already loaded by QPluginLoader::load()

    // TODO: this messes up KPluginLoader::errorString(): it will change from unknown error to could not resolve kde_plugin_version
    quint32 *version = reinterpret_cast<quint32 *>(lib.resolve("kde_plugin_version"));
    if (version) {
        d->pluginVersion = *version;
    } else {
        d->pluginVersion = ~0U;
    }
    d->pluginVersionResolved = true;

    return true;
}

QLibrary::LoadHints KPluginLoader::loadHints() const
{
    Q_D(const KPluginLoader);

    return d->loader->loadHints();
}

QJsonObject KPluginLoader::metaData() const
{
    Q_D(const KPluginLoader);

    return d->loader->metaData();
}

void KPluginLoader::setLoadHints(QLibrary::LoadHints loadHints)
{
    Q_D(KPluginLoader);

    d->loader->setLoadHints(loadHints);
}

bool KPluginLoader::unload()
{
    Q_D(KPluginLoader);

    // Even if *this* call does not unload it, another might,
    // so we err on the side of re-resolving the version.
    d->pluginVersionResolved = false;

    return d->loader->unload();
}


void KPluginLoader::forEachPlugin(const QString &directory, std::function<void(const QString &)> callback)
{
    QStringList dirsToCheck;
    if (QDir::isAbsolutePath(directory)) {
        dirsToCheck << directory;
    } else {
        const QStringList listPaths = QCoreApplication::libraryPaths();
        for (const QString &libDir : listPaths) {
            dirsToCheck << libDir + QLatin1Char('/') + directory;
        }
    }

    qCDebug(KCOREADDONS_DEBUG) << "Checking for plugins in" << dirsToCheck;

    for (const QString &dir : qAsConst(dirsToCheck)) {
        QDirIterator it(dir, QDir::Files);
        while (it.hasNext()) {
            it.next();
            if (QLibrary::isLibrary(it.fileName())) {
                callback(it.fileInfo().absoluteFilePath());
            }
        }
    }
}

QVector<KPluginMetaData> KPluginLoader::findPlugins(const QString &directory, std::function<bool(const KPluginMetaData &)> filter)
{
    QVector<KPluginMetaData> ret;
    forEachPlugin(directory, [&](const QString &pluginPath) {
        KPluginMetaData metadata(pluginPath);
        if (!metadata.isValid()) {
            return;
        }
        if (filter && !filter(metadata)) {
            return;
        }
        ret.append(metadata);
    });
    return ret;
}

QVector< KPluginMetaData > KPluginLoader::findPluginsById(const QString& directory, const QString& pluginId)
{
    auto filter = [&pluginId](const KPluginMetaData &md) -> bool
    {
        return md.pluginId() == pluginId;
    };
    return KPluginLoader::findPlugins(directory, filter);
}

QList<QObject *> KPluginLoader::instantiatePlugins(const QString &directory,
        std::function<bool(const KPluginMetaData &)> filter, QObject* parent)
{
    QList<QObject *> ret;
    QPluginLoader loader;
    const QVector<KPluginMetaData> listMetaData = findPlugins(directory, filter);
    for (const KPluginMetaData &metadata : listMetaData) {
        loader.setFileName(metadata.fileName());
        QObject* obj = loader.instance();
        if (!obj) {
            qCWarning(KCOREADDONS_DEBUG).nospace() << "Could not instantiate plugin \"" << metadata.fileName() << "\": "
                << loader.errorString();
            continue;
        }
        obj->setParent(parent);
        ret.append(obj);
    }
    return ret;
}
