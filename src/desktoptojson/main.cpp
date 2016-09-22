/******************************************************************************
 *  Copyright 2013 Sebastian Kügler <sebas@kde.org>                           *
 *  Copyright 2014 Alex Richardson <arichardson.kde@gmail.com>                *
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

#include "desktoptojson.h"

#include <QDebug>

static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "%s\n", localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(stdout, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Error: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}


int main(int argc, char** argv) {
    qInstallMessageHandler(messageOutput);
    QCoreApplication app(argc, argv);

    const QString description = QStringLiteral("Converts desktop files to json");
    app.setApplicationVersion(QStringLiteral("1.0"));

    const static auto _i = QStringLiteral("input");
    const static auto _o = QStringLiteral("output");
    const static auto _n = QStringLiteral("name");
    const static auto _c = QStringLiteral("compat");
    const static auto _s = QStringLiteral("serviceType");

    QCommandLineOption input = QCommandLineOption(QStringList() << QStringLiteral("i") << _i,
                               QStringLiteral("Read input from file"), _n);
    QCommandLineOption output = QCommandLineOption(QStringList() << QStringLiteral("o") << _o,
                                QStringLiteral("Write output to file"), _n);
    QCommandLineOption verbose = QCommandLineOption(QStringList() << QStringLiteral("verbose"),
                                QStringLiteral("Enable verbose (debug) output"));
    QCommandLineOption compat = QCommandLineOption(QStringList() << QStringLiteral("c") << _c,
                                QStringLiteral("Generate JSON that is compatible with KPluginInfo instead of the new KPluginMetaData"));
    QCommandLineOption serviceTypes = QCommandLineOption(QStringList() << QStringLiteral("s") << _s,
                                QStringLiteral("The name or full path of a KServiceType defintion .desktop file. Can be passed multiple times"), _s);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.setApplicationDescription(description);
    parser.addHelpOption();
    parser.addOption(input);
    parser.addOption(output);
    parser.addOption(verbose);
    parser.addOption(compat);
    parser.addOption(serviceTypes);

    DesktopToJson dtj(&parser, input, output, verbose, compat, serviceTypes);

    parser.process(app);
    return dtj.runMain();
}
