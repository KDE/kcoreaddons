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

#include "kpluginloader.h"

#include "kpluginfactory.h"
#include "kpluginmetadata.h"

#include <QtCore/QLibrary>
#include <QtCore/QDir>
#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtCore/QFileInfo>
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
          loader(0),
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
            qCWarning(KCOREADDONS_DEBUG) << "Error loading plugin" << pluginName.name() << d->loader->errorString() << endl
                       << "Plugin search paths are" << QCoreApplication::libraryPaths() << endl
                       << "The environment variable QT_PLUGIN_PATH might be not correctly set";
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
        return 0;
    }

    KPluginFactory *factory = qobject_cast<KPluginFactory *>(obj);

    if (factory == 0) {
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
        return 0;
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
        foreach (const QString &libDir, QCoreApplication::libraryPaths()) {
            dirsToCheck << libDir + QDir::separator() + directory;
        }
    }

    foreach (const QString &dir, dirsToCheck) {
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
    QMap<QString, QJsonObject> jsonObjects;
    const auto indexName = QStringLiteral("kpluginindex.bjson");
    QFile indexFile(directory + indexName);
    //qCDebug(KCOREADDONS_DEBUG) << "INDEX:" << !qEnvironmentVariableIsSet("KPLUGIN_SKIP_INDEX") << directory + indexName;
    if (indexFile.exists() && !qEnvironmentVariableIsSet("KPLUGIN_SKIP_INDEX")) {
        indexFile.open(QIODevice::ReadOnly);
        QJsonDocument jdoc = QJsonDocument::fromBinaryData(indexFile.readAll());
        indexFile.close();

        QSet<QString> uniqueIds;
        QJsonArray plugins = jdoc.array();
        qCDebug(KCOREADDONS_DEBUG) << "Found index!" << plugins.count();

        for (QJsonArray::const_iterator iter = plugins.constBegin(); iter != plugins.constEnd(); ++iter) {
            const QJsonObject &obj = QJsonValue(*iter).toObject();
            const QString &pluginFileName = obj.value(QStringLiteral("FileName")).toString();
            jsonObjects[pluginFileName] = obj.value(QStringLiteral("MetaData")).toObject();
        }
    }
    forEachPlugin(directory, [&](const QString &pluginPath) {
        KPluginMetaData metadata;
        if (jsonObjects.contains(pluginPath)) {
            metadata = KPluginMetaData(jsonObjects.value(pluginPath), pluginPath);
        } else {
            metadata = KPluginMetaData(pluginPath);
        }
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
    foreach (const KPluginMetaData &metadata, findPlugins(directory, filter)) {
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
