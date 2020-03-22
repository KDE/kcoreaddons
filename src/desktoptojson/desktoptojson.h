/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef DESKTOPTOJSON_H
#define DESKTOPTOJSON_H

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QString>

class QCommandLineParser;

class DesktopToJson
{
public:
    DesktopToJson(QCommandLineParser *parser, const QCommandLineOption &i,
                  const QCommandLineOption &o, const QCommandLineOption &v,
                  const QCommandLineOption &c, const QCommandLineOption &s);
    int runMain();

private:
    bool convert(const QString &src, const QString &dest, const QStringList& serviceTypes);
    void convertToJson(const QString& key, const QString &value, QJsonObject &json, QJsonObject &kplugin, int lineNr);
    void convertToCompatibilityJson(const QString &key, const QString &value, QJsonObject &json, int lineNr);
    bool resolveFiles();

    QCommandLineParser *m_parser;
    QCommandLineOption input;
    QCommandLineOption output;
    QCommandLineOption verbose;
    QCommandLineOption compat;
    QCommandLineOption serviceTypesOption;
    QString m_inFile;
    QString m_outFile;
};

#endif
