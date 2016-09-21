/******************************************************************************
 *  Copyright 2016 Sebastian Kügler <sebas@kde.org>                           *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or             *
 *  modify it under the terms of the GNU Lesser General Public                *
 *                                                                            *
 *  License as published by the Free Software Foundation; either              *
 *  version 2.1 of the License, or (at your option) version 3, or any         *
 *  later version accepted by the membership of KDE e.V. (or its              *
 *  successor approved by the membership of KDE e.V.), which shall            *
 *  act as a proxy defined in Section 6 of version 3 of the license.          *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Lesser General Public License for more details.                           *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public          *
 *  License along with this library.  If not, see                             *
 *  <http://www.gnu.org/licenses/>.                                           *
 *                                                                            *
 ******************************************************************************/

#include "kpluginindexer.h"
#include "kpluginindexer_debug.h"

#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QPluginLoader>
#include <QTextStream>

KPluginIndexer::KPluginIndexer(QCommandLineParser *parser, const QCommandLineOption &p,
                             const QCommandLineOption &u, const QCommandLineOption &c,
                             const QCommandLineOption &s)
    : m_parser(parser)
      , paths(p)
      , update(u)
      , clean(c)
      , status(s)
{
}

KPluginIndexer::KPluginIndexer()
    : m_parser(nullptr)
      , paths(QCommandLineOption(QStringLiteral("paths")))
      , update(QCommandLineOption(QStringLiteral("update")))
      , clean(QCommandLineOption(QStringLiteral("clean")))
      , status(QCommandLineOption(QStringLiteral("status")))
{
}

int KPluginIndexer::runMain()
{
    if (!resolveFiles()) {
        return 1;
    }
    if (m_parser->isSet(clean)) {
        cleanDirectoryIndex();
    }

    if (m_parser->isSet(update)) {
        foreach (const QString &dir, m_pluginDirectories) {
            createDirectoryIndex(dir);
        }
    }

    return 0;
}

bool KPluginIndexer::resolveFiles()
{
    if (m_parser && m_parser->isSet(paths)) {
        m_pluginDirectories = m_parser->values(paths);
    } else {
        foreach (const QString &libDir, QCoreApplication::libraryPaths()) {
            m_pluginDirectories << libDir;
            QDirIterator it(libDir, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
            while (it.hasNext()) {
                m_pluginDirectories << it.next();
            }
        }
    }
    qCDebug(KPI) << "Dirs" << m_pluginDirectories.count();

    return !m_pluginDirectories.isEmpty();
}

bool KPluginIndexer::createDirectoryIndex(const QString& path, const QString &dest)
{
    QJsonArray plugins;

    QPluginLoader loader;
    QDirIterator it(path, QDir::Files);
    while (it.hasNext()) {
        it.next();
        if (QLibrary::isLibrary(it.fileName())) {
            const QString pluginPath = it.fileInfo().absoluteFilePath();
            loader.setFileName(pluginPath);
            QJsonObject obj = loader.metaData();
            obj.insert(QStringLiteral("FileName"), pluginPath);
            plugins.append(obj);
        }
    }

    if (plugins.count() <= 2) {
        return true;
    }

    QString destfile = !dest.isEmpty() ? dest : m_indexFileName;
    if (!QDir::isAbsolutePath(dest)) {
        destfile = path + QLatin1Char('/') + destfile;
    }

    QDir().mkpath(QFileInfo(destfile).dir().absolutePath());
    QFile file(destfile);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open " << destfile;
        return false;
    }

    QJsonDocument jdoc;
    jdoc.setArray(plugins);
    //file.write(jdoc.toJson());
    file.write(jdoc.toBinaryData());
    qCDebug(KPI) << "Generated " << destfile << " (" << plugins.count() << " plugins)";

    return true;
}

bool KPluginIndexer::cleanDirectoryIndex(const QString& dest)
{
    bool ok = true;
    const QString indexFile = !dest.isEmpty() ? dest : m_indexFileName;
    foreach (const QString &dir, m_pluginDirectories) {
        QFileInfo fileInfo(dir, indexFile);
        if (fileInfo.exists()) {
            if (fileInfo.isWritable()) {
                QFile f(fileInfo.absoluteFilePath());
                if (!f.remove()) {
                    ok = false;
                    qCWarning(KPI) << "Cannot remove kplugin index file: " << fileInfo.absoluteFilePath();
                } else {
                    qCDebug(KPI) << "Deleted index: " << fileInfo.absoluteFilePath();
                }
            } else {
                qCWarning(KPI) << "Cannot remove kplugin index file (not writable): " << fileInfo.absoluteFilePath();
                ok = false;
            }
        }
    }
    return ok;
}

bool KPluginIndexer::isCacheUpToDate(const QString& path)
{
    const auto indexInfole = path + m_indexFileName;
    QFileInfo indexInfo(indexInfole);
    if (!indexInfo.exists()) {
        //qCDebug(KPI) << "indexInfole doesn't exist" << indexInfole;
        return false;
    }
    const QString pluginDir = indexInfo.absolutePath();
    QFileInfo pluginDirInfo(pluginDir);
    return pluginDirInfo.lastModified() <= indexInfo.lastModified();
}
