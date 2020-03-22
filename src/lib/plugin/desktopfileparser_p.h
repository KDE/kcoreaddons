/*
    SPDX-FileCopyrightText: 2013-2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

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
    QVector<CustomPropertyDefinition> m_propertyDefs;
    QByteArray m_serviceTypeName;
};

struct ServiceTypeDefinitions
{
    static ServiceTypeDefinitions fromFiles(const QStringList &paths);
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

    bool hasServiceType(const QByteArray &serviceTypeName) const;

private:
    QVector<ServiceTypeDefinition> m_definitions;
};

namespace DesktopFileParser
{
    QByteArray escapeValue(const QByteArray &input);
    QStringList deserializeList(const QString &data, char separator = ',');
    bool convert(const QString &src, const QStringList &serviceTypes, QJsonObject &json, QString *libraryPath);
    void convertToJson(const QByteArray &key, ServiceTypeDefinitions &serviceTypes, const QString &value,
                       QJsonObject &json, QJsonObject &kplugin, int lineNr);
#ifdef BUILDING_DESKTOPTOJSON_TOOL
    void convertToCompatibilityJson(const QString &key, const QString &value, QJsonObject &json, int lineNr);
    extern bool s_verbose;
    extern bool s_compatibilityMode;
#endif
}


#endif // DESKTOPFILEPARSER_H
