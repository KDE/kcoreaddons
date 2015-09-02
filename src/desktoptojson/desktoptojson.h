/******************************************************************************
 *  Copyright 2013 Sebastian Kügler <sebas@kde.org>                           *
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
