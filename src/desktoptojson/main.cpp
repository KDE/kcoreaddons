/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "desktoptojson.h"

static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    auto getFprintfS = [](auto data) {
        if (data == nullptr) {
            return "";
        }
        return data;
    };

    QByteArray localMsg = msg.toLocal8Bit();

    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "%s\n", getFprintfS(localMsg.constData()));
        break;
    case QtInfoMsg:
        fprintf(stdout, "Info: %s (%s:%u, %s)\n", getFprintfS(localMsg.constData()), getFprintfS(context.file), context.line, getFprintfS(context.function));
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", getFprintfS(localMsg.constData()), getFprintfS(context.file), context.line, getFprintfS(context.function));
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Error: %s (%s:%u, %s)\n", getFprintfS(localMsg.constData()), getFprintfS(context.file), context.line, getFprintfS(context.function));
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", getFprintfS(localMsg.constData()), getFprintfS(context.file), context.line, getFprintfS(context.function));
        abort();
    }
}

int main(int argc, char **argv)
{
    qInstallMessageHandler(messageOutput);
    QCoreApplication app(argc, argv);

    const QString description = QStringLiteral("Converts desktop files to json");
    app.setApplicationVersion(QStringLiteral("1.0"));

    const auto _i = QStringLiteral("input");
    const auto _o = QStringLiteral("output");
    const auto _n = QStringLiteral("name");
    const auto _c = QStringLiteral("compat");
    const auto _s = QStringLiteral("serviceType");

    QCommandLineOption input = QCommandLineOption(QStringList{QStringLiteral("i"), _i}, QStringLiteral("Read input from file"), _n);
    QCommandLineOption output = QCommandLineOption(QStringList{QStringLiteral("o"), _o}, QStringLiteral("Write output to file"), _n);
    QCommandLineOption verbose = QCommandLineOption(QStringList{QStringLiteral("verbose")}, QStringLiteral("Enable verbose (debug) output"));
    QCommandLineOption compat = QCommandLineOption(QStringList{QStringLiteral("c"), _c},
                                                   QStringLiteral("Generate JSON that is compatible with KPluginInfo instead of the new KPluginMetaData"));
    QCommandLineOption serviceTypes =
        QCommandLineOption(QStringList{QStringLiteral("s"), _s},
                           QStringLiteral("The name or full path of a KServiceType definition .desktop file. Can be passed multiple times"),
                           _s);
    QCommandLineOption genericDataPath =
        QCommandLineOption(QStringList{QStringLiteral("generic-data-path")},
                           QStringLiteral("Override the default search path for service types (useful when cross-compiling). Can be passed multiple times"),
                           QStringLiteral("PATH"));
    QCommandLineOption strictPathMode = QCommandLineOption(QStringList{QStringLiteral("strict-path-mode")},
                                                           QStringLiteral("Only search for service types in the explicitly listed data directories."));

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.setApplicationDescription(description);
    parser.addHelpOption();
    parser.addOption(input);
    parser.addOption(output);
    parser.addOption(verbose);
    parser.addOption(compat);
    parser.addOption(serviceTypes);
    parser.addOption(genericDataPath);
    parser.addOption(strictPathMode);

    DesktopToJson dtj(&parser, input, output, verbose, compat, serviceTypes, strictPathMode, genericDataPath);

    parser.process(app);
    return dtj.runMain();
}
