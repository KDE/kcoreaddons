/*  This file is part of the KDE project
    Copyright (C) 2014 Alex Richardson <arichardson.kde@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kpluginmetadata.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonArray>
#include <QLocale>
#include <QPluginLoader>
#include <QStringList>
#include <QDebug>

#include "kpluginloader.h"
#include "kaboutdata.h"

class KPluginMetaDataPrivate : public QSharedData
{
public:
    static bool convert(const QString &src, QJsonObject &json);
    static void convertToJson(const QString& key, const QString &value, QJsonObject &json, QJsonObject &kplugin, int lineNr);
    // This code was taken from KConfigGroupPrivate::deserializeList
    static QStringList deserializeList(const QString &data)
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

    static QByteArray escapeValue(const QByteArray& input)
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

};

KPluginMetaData::KPluginMetaData()
{
}

KPluginMetaData::KPluginMetaData(const KPluginMetaData &other)
    : m_metaData(other.m_metaData), m_fileName(other.fileName())
{
}

KPluginMetaData& KPluginMetaData::operator=(const KPluginMetaData& other)
{
    m_metaData = other.m_metaData;
    m_fileName = other.m_fileName;
    return *this;
}

KPluginMetaData::~KPluginMetaData()
{
}

KPluginMetaData::KPluginMetaData(const QString &file)
{
    if (file.endsWith(QStringLiteral(".desktop"))) {
        qDebug() << "Loading KPluginMetaData from .desktop file";
        m_fileName = QFileInfo(file).absoluteFilePath();
        KPluginMetaDataPrivate::convert(file, m_metaData);

    } else {
        QPluginLoader loader(file);
        m_fileName = QFileInfo(loader.fileName()).absoluteFilePath();
        m_metaData = loader.metaData().value(QStringLiteral("MetaData")).toObject();
    }
}

KPluginMetaData::KPluginMetaData(const QPluginLoader &loader)
{
    m_fileName = QFileInfo(loader.fileName()).absoluteFilePath();
    m_metaData = loader.metaData().value(QStringLiteral("MetaData")).toObject();
}

KPluginMetaData::KPluginMetaData(const KPluginLoader &loader)
{
    m_fileName = QFileInfo(loader.fileName()).absoluteFilePath();
    m_metaData = loader.metaData().value(QStringLiteral("MetaData")).toObject();
}

KPluginMetaData::KPluginMetaData(const QJsonObject &metaData, const QString &file)
{
    // passing QFileInfo an empty string gives the CWD, which is not what we want
    if (!file.isEmpty()) {
        m_fileName = QFileInfo(file).absoluteFilePath();
    }
    m_metaData = metaData;
}

QJsonObject KPluginMetaData::rawData() const
{
    return m_metaData;
}

QString KPluginMetaData::fileName() const
{
    return m_fileName;
}

bool KPluginMetaData::isValid() const
{
    // it can be valid even if m_fileName is empty (as long as the plugin id is set in the metadata)
    return !pluginId().isEmpty() && !m_metaData.isEmpty();
}

QJsonObject KPluginMetaData::rootObject() const
{
    return m_metaData[QStringLiteral("KPlugin")].toObject();
}

QStringList KPluginMetaData::readStringList(const QJsonObject &obj, const QString &key)
{
    const QJsonValue value = obj.value(key);
    if (value.isUndefined() || value.isObject() || value.isNull()) {
        return QStringList();
    } else if (value.isArray()) {
        return value.toVariant().toStringList();
    } else if (value.isString()) {
        return QStringList(value.toString());
    } else {
        return QStringList(value.toVariant().toString());
    }
}

QJsonValue KPluginMetaData::readTranslatedValue(const QJsonObject &jo, const QString &key, const QJsonValue &defaultValue)
{
    QString languageWithCountry = QLocale().name();
    auto it = jo.constFind(key + QLatin1Char('[') + languageWithCountry + QLatin1Char(']'));
    if (it != jo.constEnd()) {
        return it.value();
    }
    const QString language = languageWithCountry.mid(0, languageWithCountry.indexOf(QLatin1Char('_')));
    it = jo.constFind(key + QLatin1Char('[') + language + QLatin1Char(']'));
    if (it != jo.constEnd()) {
        return it.value();
    }
    // no translated value found -> check key
    it = jo.constFind(key);
    if (it != jo.constEnd()) {
        return jo.value(key);
    }
    return defaultValue;
}

QString KPluginMetaData::readTranslatedString(const QJsonObject &jo, const QString &key, const QString &defaultValue)
{
    return readTranslatedValue(jo, key, defaultValue).toString(defaultValue);
}

static inline void addPersonFromJson(const QJsonObject& obj, QList<KAboutPerson>* out) {
    const QString name = obj[QStringLiteral("Name")].toString();
    const QString task = obj[QStringLiteral("Task")].toString();
    const QString email = obj[QStringLiteral("Email")].toString();
    const QString website = obj[QStringLiteral("Website")].toString();
    const QString userName = obj[QStringLiteral("UserName")].toString();
    if (name.isEmpty()) {
        qWarning() << "Invalid plugin metadata: Attempting to create a KAboutPerson from json without 'Name' property:" << obj;
        return;
    }
    out->append(KAboutPerson(name, task, email, website, userName));

}

QList<KAboutPerson> KPluginMetaData::authors() const
{
    QList<KAboutPerson> ret;
    QJsonValue authorsValue = rootObject()[QStringLiteral("Authors")];
    if (authorsValue.isObject()) {
        // single author
        addPersonFromJson(authorsValue.toObject(), &ret);
    } else if (authorsValue.isArray()) {
        foreach (const QJsonValue &val, authorsValue.toArray()) {
            if (val.isObject()) {
                addPersonFromJson(val.toObject(), &ret);
            }
        }
    }
    return ret;
}

QString KPluginMetaData::category() const
{
    return rootObject()[QStringLiteral("Category")].toString();
}

QString KPluginMetaData::description() const
{
    return readTranslatedString(rootObject(), QStringLiteral("Description"));
}

QString KPluginMetaData::iconName() const
{
    return rootObject()[QStringLiteral("Icon")].toString();
}

QString KPluginMetaData::license() const
{
    return rootObject()[QStringLiteral("License")].toString();
}

QString KPluginMetaData::name() const
{
    return readTranslatedString(rootObject(), QStringLiteral("Name"));
}

QString KPluginMetaData::pluginId() const
{
    QJsonObject root = rootObject();
    auto nameFromMetaData = root.constFind(QStringLiteral("Id"));
    if (nameFromMetaData != root.constEnd()) {
        return nameFromMetaData.value().toString();
    }
    // passing QFileInfo an empty string gives the CWD, which is not what we want
    if (m_fileName.isEmpty()) {
        return QString();
    }
    return QFileInfo(m_fileName).baseName();
}

QString KPluginMetaData::version() const
{
    return rootObject()[QStringLiteral("Version")].toString();
}

QString KPluginMetaData::website() const
{
    return rootObject()[QStringLiteral("Website")].toString();
}

QStringList KPluginMetaData::dependencies() const
{
    return readStringList(rootObject(), QStringLiteral("Dependencies"));
}

QStringList KPluginMetaData::serviceTypes() const
{
    return readStringList(rootObject(), QStringLiteral("ServiceTypes"));
}

bool KPluginMetaData::isEnabledByDefault() const
{
    QJsonValue val = rootObject()[QStringLiteral("EnabledByDefault")];
    if (val.isBool()) {
        return val.toBool();
    } else if (val.isString()) {
        return val.toString() == QLatin1String("true");
    }
    return false;
}

QString KPluginMetaData::value(const QString& key, const QString& defaultValue) const
{
    return m_metaData.value(key).toString(defaultValue);
}

bool KPluginMetaData::operator==(const KPluginMetaData& other) const
{
    return m_fileName == other.m_fileName && m_metaData == other.m_metaData;
}

QObject* KPluginMetaData::instantiate() const
{
    return QPluginLoader(m_fileName).instance();
}


void KPluginMetaDataPrivate::convertToJson(const QString &key, const QString &value, QJsonObject &json, QJsonObject &kplugin, int lineNr)
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

bool KPluginMetaDataPrivate::convert(const QString& src, QJsonObject &json)
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

