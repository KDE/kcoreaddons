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


#include "desktopfileparser.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>

#include <QDebug>


    // This code was taken from KConfigGroupPrivate::deserializeList
QStringList DesktopFileParser::deserializeList(const QString &data)
{
    if (data.isEmpty()) {
        return QStringList();
    }
    if (data == QLatin1String("\\0")) {
        return QStringList(QString());
    }
    QStringList value;
    QString val;
    val.reserve(data.size());
    bool quoted = false;
    for (int p = 0; p < data.length(); p++) {
        if (quoted) {
            val += data[p];
            quoted = false;
        } else if (data[p].unicode() == '\\') {
            quoted = true;
        } else if (data[p].unicode() == ',') {
            val.squeeze(); // release any unused memory
            value.append(val);
            val.clear();
            val.reserve(data.size() - p);
        } else {
            val += data[p];
        }
    }
    value.append(val);
    return value;
}

QByteArray DesktopFileParser::escapeValue(const QByteArray& input)
{
    // we could do this in place, but this code is simpler
    // this tool is probably only transitional, so no need to optimize
    QByteArray result;
    for (int i = 0; i < input.length(); ++i) {
        if (input[i] != '\\') {
            result.append(input[i]);
        } else {
            if (i + 1 >= input.length()) {
                // just append the backslash if we are at end of line
                result.append(input[i]);
                break;
            }
            i++; // consume next character
            char nextChar = input[i];
            switch (nextChar) {
            case 's':
                result.append(' ');
                break;
            case 'n':
                result.append('\n');
                break;
            case 't':
                result.append('\t');
                break;
            case 'r':
                result.append('\r');
                break;
            case '\\':
                result.append('\\');
                break;
            default:
                result.append('\\');
                result.append(nextChar); // just ignore the escape sequence
            }
        }
    }
    return result;
}

void DesktopFileParser::convertToJson(const QString &key, const QString &value, QJsonObject &json, QJsonObject &kplugin, int lineNr)
{
    bool m_verbose = false; // FIXME remove
    /* The following keys are recognized (and added to a "KPlugin" object):

        Icon=mypluginicon
        Type=Service
        ServiceTypes=KPluginInfo

        Name=User Visible Name (translatable)
        Comment=Description of what the plugin does (translatable)

        X-KDE-PluginInfo-Author=Author's Name
        X-KDE-PluginInfo-Email=author@foo.bar
        X-KDE-PluginInfo-Name=internalname
        X-KDE-PluginInfo-Version=1.1
        X-KDE-PluginInfo-Website=http://www.plugin.org/
        X-KDE-PluginInfo-Category=playlist
        X-KDE-PluginInfo-Depends=plugin1,plugin3
        X-KDE-PluginInfo-License=GPL
        X-KDE-PluginInfo-EnabledByDefault=true
    */
    if (key == QLatin1String("Icon")) {
        kplugin[QStringLiteral("Icon")] = value;
    } else if (key == QLatin1String("X-KDE-PluginInfo-Name")) {
        kplugin[QStringLiteral("Id")] = value;
    } else if (key == QLatin1String("X-KDE-PluginInfo-Category")) {
        kplugin[QStringLiteral("Category")] = value;
    } else if (key == QLatin1String("X-KDE-PluginInfo-License")) {
        kplugin[QStringLiteral("License")] = value;
    } else if (key == QLatin1String("X-KDE-PluginInfo-Version")) {
        kplugin[QStringLiteral("Version")] = value;
    } else if (key == QLatin1String("X-KDE-PluginInfo-Website")) {
        kplugin[QStringLiteral("Website")] = value;
    } else if (key == QLatin1String("X-KDE-PluginInfo-Depends")) {
        kplugin[QStringLiteral("Dependencies")] = QJsonArray::fromStringList(deserializeList(value));
    } else if (key == QLatin1String("X-KDE-ServiceTypes") || key == QLatin1String("ServiceTypes")) {
        // some .desktop files still use the legacy ServiceTypes= key
        kplugin[QStringLiteral("ServiceTypes")] = QJsonArray::fromStringList(deserializeList(value));
    } else if (key == QLatin1String("X-KDE-PluginInfo-EnabledByDefault")) {
        bool boolValue = false;
        // should only be lower case, but be tolerant here
        if (value.toLower() == QLatin1String("true")) {
            boolValue = true;
        } else {
            if (value.toLower() != QLatin1String("false")) {
                //qWarning() << "Warning: " << m_inFile << ':' << lineNr << ": Expected boolean value for key \""
                //    << key << "\" but got \"" << value << "\" instead." << endl;
            }
        }
        kplugin[QStringLiteral("EnabledByDefault")] = boolValue;
    } else if (key == QLatin1String("X-KDE-PluginInfo-Author")) {
        QJsonObject authorsObject = kplugin.value(QStringLiteral("Authors")).toArray().at(0).toObject();
        // if the authors object doesn't exist yet this will create it
        authorsObject[QStringLiteral("Name")] = value;
        QJsonArray array;
        array.append(authorsObject);
        kplugin[QStringLiteral("Authors")] = array;
    } else if (key == QLatin1String("X-KDE-PluginInfo-Email")) {
        QJsonObject authorsObject = kplugin.value(QStringLiteral("Authors")).toArray().at(0).toObject();
        // if the authors object doesn't exist yet this will create it
        authorsObject[QStringLiteral("Email")] = value;
        QJsonArray array;
        array.append(authorsObject);
        kplugin[QStringLiteral("Authors")] = array;
    } else if (key == QLatin1String("Name") || key.startsWith(QLatin1String("Name["))) {
        // TODO: also handle GenericName? does that make any sense, or is X-KDE-PluginInfo-Category enough?
        kplugin[key] = value;
    } else if (key == QLatin1String("Comment")) {
        kplugin[QStringLiteral("Description")] = value;
    } else if (key.startsWith(QLatin1String("Comment["))) {
        kplugin[QStringLiteral("Description") + key.mid(strlen("Comment"))] = value;
    } else if (key == QLatin1String("Hidden")) {
        if (m_verbose) {
            qWarning() << "Warning: Hidden= key found in desktop file, this makes no sense"
                " with metadata inside the plugin." << endl;
        }
        kplugin[key] = (value.toLower() == QLatin1String("true"));
    } else if (key == QLatin1String("Exec") || key == QLatin1String("Type") || key == QLatin1String("X-KDE-Library")) {
        // Exec= doesn't make sense here, however some .desktop files (like e.g. in kdevelop) have a dummy value here
        // also the Type=Service entry is no longer needed
        // X-KDE-Library is also not needed since we already have the library to read this metadata
        if (m_verbose) {
            qDebug() << "Not converting key " << key << "=" << value << endl;
        }
    } else {
        // unknown key, just add it to the root object
        json[key] = value;
    }
}

bool DesktopFileParser::convert(const QString& src, QJsonObject &json, QString *libraryPath)
{
    bool m_verbose = false; // FIXME remove
    QFile df(src);
    if (!df.open(QFile::ReadOnly)) {
        qWarning() << "Error: Failed to open " << src << endl;
        return false;
    }
    int lineNr = 0;
    // we only convert data inside the [Desktop Entry] group
    while (!df.atEnd()) {
        QByteArray line = df.readLine().trimmed();
        lineNr++;
        if (line == "[Desktop Entry]") {
            if (m_verbose) {
                qDebug() << "Found desktop group in line " << lineNr << endl;
            }
            break;
        }
    }
    if (df.atEnd()) {
        qWarning() << "Error: Could not find [Desktop Entry] group in " << src << endl;
        return false;
    }


    //QJsonObject json;
    QJsonObject kplugin; // the "KPlugin" key of the metadata
    while (!df.atEnd()) {
        const QByteArray line = df.readLine().trimmed();
        lineNr++;
        if (line.isEmpty()) {
            if (m_verbose) {
                qDebug() << "Line " << lineNr << ": empty" << endl;
            }
            continue;
        }
        if (line.startsWith('#')) {
            if (m_verbose) {
                qDebug() << "Line " << lineNr << ": comment" << endl;
            }
            continue; // skip comments
        }
        if (line.startsWith('[')) {
            // start of new group -> doesn't interest us anymore
            if (m_verbose) {
                qDebug() << "Line " << lineNr << ": start of new group " << line << endl;
            }
            break;
        }
        // must have form key=value now
        const int equalsIndex = line.indexOf('=');
        if (equalsIndex == -1) {
            qWarning() << "Warning: " << src << ':' << lineNr << ": Line is neither comment nor group "
                "and doesn't contain an '=' character: \"" << line << '\"' << endl;
            continue;
        }
        // trim key and value to remove spaces around the '=' char
        const QString key = QString::fromLatin1(line.mid(0, equalsIndex).trimmed());
        const QByteArray valueRaw = line.mid(equalsIndex + 1).trimmed();
        const QByteArray valueEscaped = escapeValue(valueRaw);
        const QString value = QString::fromUtf8(valueEscaped);
        if (m_verbose) {
            qDebug() << "Line " << lineNr << ": key=\"" << key << "\", value=\"" << value << '"' << endl;
            if (valueEscaped != valueRaw) {
                qDebug() << "Line " << lineNr << " contained escape sequences" << endl;
            }
        }
        convertToJson(key, value, json, kplugin, lineNr);
        if (key == QLatin1String("X-KDE-Library")) {
            *libraryPath = value;
        }
    }
    json[QStringLiteral("KPlugin")] = kplugin;
    /*
    QJsonDocument jdoc;
    jdoc.setObject(json);
    QFile file(dest);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open " << dest << endl;
        return false;
    }

    file.write(jdoc.toJson());
    qDebug() << "Generated " << dest << endl;
    */
    return true;
}

