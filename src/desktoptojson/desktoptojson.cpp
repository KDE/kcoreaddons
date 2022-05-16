/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "desktoptojson.h"

#include "../lib/kcoreaddons_export.h"
#include "../lib/plugin/desktopfileparser_p.h"
#include "desktoptojson_debug.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

bool DesktopFileParser::s_verbose = false;
bool DesktopFileParser::s_compatibilityMode = false;

int DesktopToJson::runMain()
{
    if (!m_parser->isSet(input)) {
        m_parser->showHelp(1);
        return 1;
    }
    if (m_parser->isSet(verbose)) {
        DesktopFileParser::s_verbose = true;
    }
    if (m_parser->isSet(compat)) {
        DesktopFileParser::s_compatibilityMode = true;
    }
    if (!resolveFiles()) {
        qCCritical(DESKTOPPARSER) << "Failed to resolve filenames" << m_inFile << m_outFile;
        return 1;
    }

    // #pragma message("TODO: make it an error if one of the service type files is invalid or not found")
    const QStringList serviceTypes = m_parser->values(serviceTypesOption);
    QStringList searchPaths = m_parser->values(genericDataPathOption);
    if (!m_parser->isSet(strictPathMode)) {
        // In non-strict mode, we also search the default (host) generic data location.
        searchPaths.append(QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation));
    } else if (searchPaths.empty()) {
        qCCritical(DESKTOPPARSER) << "Strict path mode enabled but no service types search path passed";
        return 1;
    }
    return convert(m_inFile, m_outFile, serviceTypes, searchPaths) ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool DesktopToJson::resolveFiles()
{
    if (m_parser->isSet(input)) {
        m_inFile = m_parser->value(input);
        const QFileInfo fi(m_inFile);
        if (!fi.exists()) {
            qCCritical(DESKTOPPARSER) << "File not found: " << m_inFile;
            return false;
        }
        if (!fi.isAbsolute()) {
            m_inFile = fi.absoluteFilePath();
        }
    }

    if (m_parser->isSet(output)) {
        m_outFile = m_parser->value(output);
    } else if (!m_inFile.isEmpty()) {
        m_outFile = m_inFile;
        m_outFile.replace(QStringLiteral(".desktop"), QStringLiteral(".json"));
    }

    return m_inFile != m_outFile && !m_inFile.isEmpty() && !m_outFile.isEmpty();
}

void DesktopFileParser::convertToCompatibilityJson(const QString &key, const QString &value, QJsonObject &json, int lineNr)
{
    // XXX: Hidden=true doesn't make sense with json plugins since the metadata is inside the .so
    static const QStringList boolkeys = QStringList{
        QStringLiteral("Hidden"),
        QStringLiteral("X-KDE-PluginInfo-EnabledByDefault"),
    };
    static const QStringList stringlistkeys = QStringList
    {
        QStringLiteral("X-KDE-ServiceTypes"),
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 79)
            QStringLiteral("X-KDE-PluginInfo-Depends"),
#endif
    };
    if (boolkeys.contains(key)) {
        // should only be lower case, but be tolerant here
        if (value.toLower() == QLatin1String("true")) {
            json[key] = true;
        } else {
            if (value.toLower() != QLatin1String("false")) {
                qCWarning(DESKTOPPARSER).nospace() << "Expected boolean value for key \"" << key << "\" at line " << lineNr << "but got \"" << value
                                                   << "\" instead.";
            }
            json[key] = false;
        }
    } else if (stringlistkeys.contains(key)) {
        json[key] = QJsonArray::fromStringList(DesktopFileParser::deserializeList(value));
    } else {
        json[key] = value;
    }
}

bool DesktopToJson::convert(const QString &src, const QString &dest, const QStringList &serviceTypes, const QStringList &searchPaths)
{
    QJsonObject json;
    DesktopFileParser::convert(src, serviceTypes, json, nullptr, searchPaths);

    if (DesktopFileParser::s_compatibilityMode) {
        Q_ASSERT(json.value(QStringLiteral("KPlugin")).toObject().isEmpty());
        json.remove(QStringLiteral("KPlugin"));
    }
    QJsonDocument jdoc;
    jdoc.setObject(json);

    QFile file(dest);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCCritical(DESKTOPPARSER) << "Failed to open " << dest;
        return false;
    }

    file.write(jdoc.toJson());
    qCDebug(DESKTOPPARSER) << "Generated " << dest;
    return true;
}
