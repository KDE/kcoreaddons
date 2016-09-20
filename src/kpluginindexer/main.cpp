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

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    const QString description = QStringLiteral("Maintains an index for Qt plugin metadata to speed up plugin lookup");
    app.setApplicationVersion(QStringLiteral("1.0"));

    const static auto _c = QStringLiteral("clean");
    const static auto _s = QStringLiteral("status");
    const static auto _u = QStringLiteral("update");
    const static auto _p = QStringLiteral("path");

    QCommandLineOption clean = QCommandLineOption(QStringList() << QStringLiteral("c") << _c,
                               QStringLiteral("Remove index files"));
    QCommandLineOption status = QCommandLineOption(QStringList() << QStringLiteral("s") << _s,
                                QStringLiteral("Show index status"));
    QCommandLineOption update = QCommandLineOption(QStringList() << QStringLiteral("u") << _u,
                                QStringLiteral("Update the plugin index"));
    QCommandLineOption paths = QCommandLineOption(QStringList() << QStringLiteral("p") << _p,
                                QStringLiteral("Plugin paths to operate on. Can be passed multiple times"), QStringLiteral("plugin path"));

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.setApplicationDescription(description);
    parser.addHelpOption();
    parser.addOption(paths);
    parser.addOption(update);
    parser.addOption(clean);
    parser.addOption(status);

    parser.process(app);
    KPluginIndexer indexer(&parser, paths, update, clean, status);

    return indexer.runMain();
}
