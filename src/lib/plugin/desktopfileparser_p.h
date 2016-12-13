/******************************************************************************
 *  Copyright 2013-2014 Sebastian Kügler <sebas@kde.org>                      *
 *  Copyright 2014 Alex Richardson <arichardson.kde@gmail.com>                *
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

#ifndef DESKTOPFILEPARSER_H
#define DESKTOPFILEPARSER_H

#include <QByteArray>
#include <QLoggingCategory>
#include <QVector>
class QJsonObject;
class QJsonValue;

Q_DECLARE_LOGGING_CATEGORY(DESKTOPPARSER)

struct CustomPropertyDefinition;
struct ServiceTypeDefinition
{
    ServiceTypeDefinition();

    static ServiceTypeDefinition fromFiles(const QStringList &paths);
    /**
     * @return @p value converted to the correct JSON type.
     * If there is no custom property definition for @p key this will simply return the string value
     */
    QJsonValue parseValue(const QByteArray &key, const QString &value) const;

    /**
     * Parses the service file in @p path and extracts its definitions
     *
     * @returns whether the action could be performed
     */
    bool addFile(const QString &path);

private:
    QVector<CustomPropertyDefinition> m_definitions;
};

namespace DesktopFileParser
{
    QByteArray escapeValue(const QByteArray &input);
    QStringList deserializeList(const QString &data, char separator = ',');
    bool convert(const QString &src, const QStringList &serviceTypes, QJsonObject &json, QString *libraryPath);
    void convertToJson(const QByteArray &key, ServiceTypeDefinition &serviceTypes, const QString &value,
                       QJsonObject &json, QJsonObject &kplugin, int lineNr);
#ifdef BUILDING_DESKTOPTOJSON_TOOL
    void convertToCompatibilityJson(const QString &key, const QString &value, QJsonObject &json, int lineNr);
    extern bool s_verbose;
    extern bool s_compatibilityMode;
#endif
}


#endif // DESKTOPFILEPARSER_H
