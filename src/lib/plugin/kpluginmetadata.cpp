/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kpluginmetadata.h"
#include "desktopfileparser_p.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocale>
#include <QMimeDatabase>
#include <QPluginLoader>
#include <QStringList>
#include "kcoreaddons_debug.h"

#include "kpluginloader.h"
#include "kaboutdata.h"

class KPluginMetaDataPrivate : public QSharedData
{
public:
    QString metaDataFileName;
};

KPluginMetaData::KPluginMetaData()
{
}

KPluginMetaData::KPluginMetaData(const KPluginMetaData &other)
    : m_metaData(other.m_metaData), m_fileName(other.fileName()), d(other.d)
{
}

KPluginMetaData &KPluginMetaData::operator=(const KPluginMetaData &other)
{
    m_metaData = other.m_metaData;
    m_fileName = other.m_fileName;
    d = other.d;
    return *this;
}

KPluginMetaData::~KPluginMetaData()
{
}

KPluginMetaData::KPluginMetaData(const QString &file)
{
    if (file.endsWith(QLatin1String(".desktop"))) {
        loadFromDesktopFile(file, QStringList());
    } else if (file.endsWith(QLatin1String(".json"))) {
        d = new KPluginMetaDataPrivate;
        QFile f(file);
        bool b = f.open(QIODevice::ReadOnly);
        if (!b) {
            qCWarning(KCOREADDONS_DEBUG) << "Couldn't open" << file;
            return;
        }

        QJsonParseError error;
        m_metaData = QJsonDocument::fromJson(f.readAll(), &error).object();
        if (error.error) {
            qCWarning(KCOREADDONS_DEBUG) << "error parsing" << file << error.errorString();
        }
        m_fileName = file;
        d->metaDataFileName = file;
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
    m_fileName = file;
    m_metaData = metaData;
}

KPluginMetaData::KPluginMetaData(const QJsonObject &metaData, const QString &pluginFile, const QString &metaDataFile)
{
    m_fileName = pluginFile;
    m_metaData = metaData;
    if (!metaDataFile.isEmpty()) {
        d = new KPluginMetaDataPrivate;
        d->metaDataFileName = metaDataFile;
    }
}

KPluginMetaData KPluginMetaData::fromDesktopFile(const QString &file, const QStringList &serviceTypes)
{
    KPluginMetaData result;
    result.loadFromDesktopFile(file, serviceTypes);
    return result;
}

void KPluginMetaData::loadFromDesktopFile(const QString &file, const QStringList &serviceTypes)
{
    QString libraryPath;
    if (!DesktopFileParser::convert(file, serviceTypes, m_metaData, &libraryPath)) {
        Q_ASSERT(!isValid());
        return; // file could not be parsed for some reason, leave this object invalid
    }
    d = new KPluginMetaDataPrivate;
    d->metaDataFileName = QFileInfo(file).absoluteFilePath();
    if (!libraryPath.isEmpty()) {
        // this was a plugin with a shared library
        m_fileName = libraryPath;
    } else {
        // no library, make filename point to the .desktop file
        m_fileName = d->metaDataFileName;
    }
}

QJsonObject KPluginMetaData::rawData() const
{
    return m_metaData;
}

QString KPluginMetaData::fileName() const
{
    return m_fileName;
}

QString KPluginMetaData::metaDataFileName() const
{
    return d ? d->metaDataFileName : m_fileName;
}


bool KPluginMetaData::isValid() const
{
    // it can be valid even if m_fileName is empty (as long as the plugin id is set in the metadata)
    return !pluginId().isEmpty() && !m_metaData.isEmpty();
}

bool KPluginMetaData::isHidden() const
{
    return rootObject()[QStringLiteral("Hidden")].toBool();
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
    } else {
        QString asString = value.isString() ? value.toString() : value.toVariant().toString();
        if (asString.isEmpty()) {
            return QStringList();
        }
        const QString id = obj.value(QStringLiteral("KPlugin")).toObject().value(QStringLiteral("Id")).toString();
        qCWarning(KCOREADDONS_DEBUG) << "Expected JSON property" << key << "to be a string list."
            " Treating it as a list with a single entry:" << asString << id.toLatin1().constData();
        return QStringList(asString);
    }
}

QJsonValue KPluginMetaData::readTranslatedValue(const QJsonObject &jo, const QString &key, const QJsonValue &defaultValue)
{
    QString languageWithCountry = QLocale().name();
    auto it = jo.constFind(key + QLatin1Char('[') + languageWithCountry + QLatin1Char(']'));
    if (it != jo.constEnd()) {
        return it.value();
    }
    const QStringRef language = languageWithCountry.midRef(0, languageWithCountry.indexOf(QLatin1Char('_')));
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

static inline void addPersonFromJson(const QJsonObject &obj, QList<KAboutPerson>* out) {
    KAboutPerson person = KAboutPerson::fromJSON(obj);
    if (person.name().isEmpty()) {
        qCWarning(KCOREADDONS_DEBUG) << "Invalid plugin metadata: Attempting to create a KAboutPerson from json without 'Name' property:" << obj;
        return;
    }
    out->append(person);
}

static QList<KAboutPerson> aboutPersonFromJSON(const QJsonValue &people)
{
    QList<KAboutPerson> ret;
    if (people.isObject()) {
        // single author
        addPersonFromJson(people.toObject(), &ret);
    } else if (people.isArray()) {
        const QJsonArray peopleArray =  people.toArray();
        for (const QJsonValue &val : peopleArray) {
            if (val.isObject()) {
                addPersonFromJson(val.toObject(), &ret);
            }
        }
    }
    return ret;
}

QList<KAboutPerson> KPluginMetaData::authors() const
{
    return aboutPersonFromJSON(rootObject()[QStringLiteral("Authors")]);
}

QList<KAboutPerson> KPluginMetaData::translators() const
{
    return aboutPersonFromJSON(rootObject()[QStringLiteral("Translators")]);
}

QList<KAboutPerson> KPluginMetaData::otherContributors() const
{
    return aboutPersonFromJSON(rootObject()[QStringLiteral("OtherContributors")]);
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

QString KPluginMetaData::copyrightText() const
{
    return readTranslatedString(rootObject(), QStringLiteral("Copyright"));
}

QString KPluginMetaData::extraInformation() const
{
    return readTranslatedString(rootObject(), QStringLiteral("ExtraInformation"));
}

QString KPluginMetaData::pluginId() const
{
    QJsonObject root = rootObject();
    auto nameFromMetaData = root.constFind(QStringLiteral("Id"));
    if (nameFromMetaData != root.constEnd()) {
        const QString id = nameFromMetaData.value().toString();
        if (!id.isEmpty()) {
            return id;
        }
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

QStringList KPluginMetaData::mimeTypes() const
{
    return readStringList(rootObject(), QStringLiteral("MimeTypes"));
}

bool KPluginMetaData::supportsMimeType(const QString &mimeType) const
{
    QMimeDatabase db;
    const QMimeType mime = db.mimeTypeForName(mimeType);
    if (!mime.isValid()) {
        return false;
    }

    const QStringList mimes = mimeTypes();
    auto inherits = [&](const QString &supportedMimeName) {
        return mime.inherits(supportedMimeName);
    };
    return std::find_if(mimes.begin(), mimes.end(), inherits) != mimes.end();
}

QStringList KPluginMetaData::formFactors() const
{
    return readStringList(rootObject(), QStringLiteral("FormFactors"));
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

int KPluginMetaData::initialPreference() const
{
    return rootObject()[QStringLiteral("InitialPreference")].toInt();
}

QString KPluginMetaData::value(const QString &key, const QString &defaultValue) const
{
    const QJsonValue value = m_metaData.value(key);
    if (value.isString()) {
        return value.toString(defaultValue);
    } else if (value.isArray()) {
        qCWarning(KCOREADDONS_DEBUG) << "Expected JSON property" << key << "to be a single string."
            " but it is a stringlist";
        const QStringList list = value.toVariant().toStringList();
        if (list.isEmpty()) {
            return defaultValue;
        }
        return list.join(QChar::fromLatin1(','));
    } else if (value.isBool()) {
        qCWarning(KCOREADDONS_DEBUG) << "Expected JSON property" << key << "to be a single string."
            " but it is a bool";
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    }
    return defaultValue;
}

bool KPluginMetaData::operator==(const KPluginMetaData &other) const
{
    return m_fileName == other.m_fileName && m_metaData == other.m_metaData;
}

QObject* KPluginMetaData::instantiate() const
{
    return QPluginLoader(m_fileName).instance();
}

