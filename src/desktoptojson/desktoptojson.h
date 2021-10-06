/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef DESKTOPTOJSON_H
#define DESKTOPTOJSON_H

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QString>

class QCommandLineParser;

class DesktopToJson
{
public:
    DesktopToJson(QCommandLineParser *parser,
                  const QCommandLineOption &i,
                  const QCommandLineOption &o,
                  const QCommandLineOption &v,
                  const QCommandLineOption &c,
                  const QCommandLineOption &s,
                  const QCommandLineOption &strictPathMode,
                  const QCommandLineOption &searchPaths)
        : m_parser(parser)
        , input(i)
        , output(o)
        , verbose(v)
        , compat(c)
        , serviceTypesOption(s)
        , strictPathMode(strictPathMode)
        , genericDataPathOption(searchPaths)
    {
    }
    int runMain();

private:
    bool convert(const QString &src, const QString &dest, const QStringList &serviceTypes, const QStringList &searchPaths);
    void convertToJson(const QString &key, const QString &value, QJsonObject &json, QJsonObject &kplugin, int lineNr);
    void convertToCompatibilityJson(const QString &key, const QString &value, QJsonObject &json, int lineNr);
    bool resolveFiles();

    QCommandLineParser *const m_parser;
    const QCommandLineOption input;
    const QCommandLineOption output;
    const QCommandLineOption verbose;
    const QCommandLineOption compat;
    const QCommandLineOption serviceTypesOption;
    const QCommandLineOption strictPathMode;
    const QCommandLineOption genericDataPathOption;
    QString m_inFile;
    QString m_outFile;
};

#endif
