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

#include <klocalizedstring.h>
#include "kpluginfactory.h"

#include <QtCore/QLibrary>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QDebug>
#include <QCoreApplication>

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

    // NB: should only be called from constructors (does not update name
    //     or pluginVersion or pluginVersionResolved).
    void setFileName(const QString &fileName)
    {
        Q_ASSERT(loader);

        const QString pluginLocation = KPluginLoader::findPlugin(fileName);

        if (pluginLocation.isEmpty()) {
            errorString = i18n(
                "Could not find plugin '%1' for application '%2'",
                fileName,
                QCoreApplication::instance()->applicationName());
        } else {
            loader->setFileName(pluginLocation);
        }
    }

    KPluginLoader *q_ptr;
    const QString name;
    quint32 pluginVersion;
    QString errorString;
    bool pluginVersionResolved;
    QPluginLoader *loader;
};

inline QString makeLibName(const QString &libname)
{
#if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
    if (!libname.endsWith(QLatin1String(".dll"))) {
        return libname + QLatin1String(".dll");
    }
    return libname;
#else
    int pos = libname.lastIndexOf(QLatin1Char('/'));
    if (pos < 0) {
        pos = 0;
    }
    if (libname.indexOf(QLatin1Char('.'), pos) < 0) {
        const char *const extList[] = { ".so", ".dylib", ".bundle", ".sl" };
        for (uint i = 0; i < sizeof(extList) / sizeof(*extList); ++i) {
            const QString lib = libname + QString::fromLatin1(extList[i]);
            if (QLibrary::isLibrary(lib)) {
                return lib;
            }
        }
    }
    return libname;
#endif
}

#ifdef Q_CC_MSVC
static QString removeLibPrefix(const QString &libname)
{
    return libname.startsWith(QStringLiteral("lib")) ? libname.mid(3) : libname;
}
#endif

QString KPluginLoader::findPlugin(const QString &name)
{
    // Convert name to a valid platform libname
    QString libname = makeLibName(name);
    QFileInfo fileinfo(name);
    bool hasPrefix = fileinfo.fileName().startsWith(QLatin1String("lib"));
    bool kdeinit = fileinfo.fileName().startsWith(QLatin1String("libkdeinit5_"));

    if (hasPrefix && !kdeinit) {
        qDebug() << "plugins should not have a 'lib' prefix:" << libname;
    }
#ifdef Q_CC_MSVC
    // first remove the 'lib' prefix in front of windows plugins
    libname = removeLibPrefix(libname);
#endif

    // If it is a absolute path just return it
    if (!QDir::isRelativePath(libname)) {
        return libname;
    }

#if 0
    // TEMPORARY HACK
    Q_FOREACH (const QString &path, QFile::decodeName(qgetenv("LD_LIBRARY_PATH")).split(QLatin1Char(':'), QString::SkipEmptyParts)) {
        QString libfile = path + QLatin1String("/kde5/") + libname;
        if (QFile::exists(libfile)) {
            //qDebug() << "Looking at" << libfile << ": FOUND!";
            return libfile;
        }
        //qDebug() << "Looking at" << libfile << ": doesn't exist";
        libfile = path + QLatin1String("/") + libname;
        if (QFile::exists(libfile)) {
            if (!kdeinit) {
                qDebug() << "library" << libname << "not found under 'module' but under 'lib'";
            }
            return libfile;
        }
    }
#endif

    // Ask Qt for the list of based paths containing plugins
    Q_FOREACH (const QString &path, QCoreApplication::libraryPaths()) {
        // Check for kde modules/plugins?
        QString libfile = path + QLatin1String("/kf5/") + libname;
        if (QFile::exists(libfile)) {
            //qDebug() << "Looking at" << libfile << ": FOUND!";
            return libfile;
        }
        //qDebug() << "Looking at" << libfile << ": doesn't exist";

#if 0 // old code, not sure how to port
        // Now look where they don't belong but sometimes are
#ifndef Q_CC_MSVC
        if (!hasPrefix) {
            libname = fileinfo.path() + QLatin1String("/lib") + fileinfo.fileName();
        }
#endif
#endif

        libfile = path + QLatin1String("/") + libname;
        if (QFile::exists(libfile)) {
            if (!kdeinit) {
                qDebug() << "library" << libname << "not found under 'module' but under 'lib'";
            }
            return libfile;
        }
    }

    // Nothing found
    return QString();
}

KPluginLoader::KPluginLoader(const QString &plugin, QObject *parent)
    : QObject(parent),
      d_ptr(new KPluginLoaderPrivate(plugin))
{
    d_ptr->q_ptr = this;
    Q_D(KPluginLoader);

    d->loader = new QPluginLoader(this);
    d->setFileName(plugin);
}

KPluginLoader::KPluginLoader(const KPluginName &pluginName, QObject *parent)
    : QObject(parent),
      d_ptr(new KPluginLoaderPrivate(pluginName.name()))
{
    d_ptr->q_ptr = this;
    Q_D(KPluginLoader);

    d->loader = new QPluginLoader(this);

    if (pluginName.isValid()) {
        d->setFileName(pluginName.name());
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
        d->errorString = i18n("The library %1 does not offer a KPluginFactory.", d->name);
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

