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

#include <QtCore/QLibrary>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QDebug>
#include <QCoreApplication>

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
          pluginVersion(~0U),
          pluginVersionResolved(false),
          loader(0)
    {}
    ~KPluginLoaderPrivate()
    {}

    KPluginLoader *q_ptr;
    const QString name;
    quint32 pluginVersion;
    QString errorString;
    bool pluginVersionResolved;
    QPluginLoader *loader;
};

QString KPluginLoader::findPlugin(const QString &name)
{
    // We just defer to Qt; unfortunately, QPluginLoader's searching code is not
    // accessible without creating a QPluginLoader object.
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

    if (!load()) {
        return 0;
    }

    QObject *obj = instance();

    if (!obj) {
        return 0;
    }

    KPluginFactory *factory = qobject_cast<KPluginFactory *>(obj);

    if (factory == 0) {
        qDebug() << "Expected a KPluginFactory, got a" << obj->metaObject()->className();
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

    quint32 *version = (quint32 *) lib.resolve("kde_plugin_version");
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

