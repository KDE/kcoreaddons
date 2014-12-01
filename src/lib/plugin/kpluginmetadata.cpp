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
    QPluginLoader loader(file);
    m_fileName = QFileInfo(loader.fileName()).absoluteFilePath();
    m_metaData = loader.metaData().value(QStringLiteral("MetaData")).toObject();
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
    m_fileName = file;
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
