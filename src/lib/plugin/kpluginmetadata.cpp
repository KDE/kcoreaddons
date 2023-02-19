/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpluginmetadata.h"
#include "desktopfileparser_p.h"
#include "kstaticpluginhelpers_p.h"

#include "kcoreaddons_debug.h"
#include "kjsonutils.h"
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocale>
#include <QMimeDatabase>
#include <QPluginLoader>
#include <QStandardPaths>

#include "kaboutdata.h"
#include "kpluginfactory.h"
#include "kpluginloader.h"

#include <optional>

class KPluginMetaDataPrivate : public QSharedData
{
public:
    // If we want to load a file, but it does not exist we want to keep the requested file name for logging
    QString m_requestedFileName;
    QString metaDataFileName;
    // TODO KF6, use std::optional for m_metaData, currently this is a member of the exported class
    // see https://phabricator.kde.org/T14958
    KPluginMetaData::KPluginMetaDataOption m_option = KPluginMetaData::DoNotAllowEmptyMetaData;
    std::optional<QStaticPlugin> staticPlugin = std::nullopt;
    static void forEachPlugin(const QString &directory, std::function<void(const QString &)> callback)
    {
        QStringList dirsToCheck;
#ifdef Q_OS_ANDROID
        dirsToCheck << QCoreApplication::libraryPaths();
#else
        if (QDir::isAbsolutePath(directory)) {
            dirsToCheck << directory;
        } else {
            dirsToCheck = QCoreApplication::libraryPaths();
            const QString appDirPath = QCoreApplication::applicationDirPath();
            dirsToCheck.removeAll(appDirPath);
            dirsToCheck.prepend(appDirPath);

            for (QString &libDir : dirsToCheck) {
                libDir += QLatin1Char('/') + directory;
            }
        }
#endif

        qCDebug(KCOREADDONS_DEBUG) << "Checking for plugins in" << dirsToCheck;

        for (const QString &dir : std::as_const(dirsToCheck)) {
            QDirIterator it(dir, QDir::Files);
            while (it.hasNext()) {
                it.next();
#ifdef Q_OS_ANDROID
                QString prefix(QLatin1String("libplugins_") + QString(directory).replace(QLatin1Char('/'), QLatin1String("_")));
                if (!prefix.endsWith(QLatin1Char('_'))) {
                    prefix.append(QLatin1Char('_'));
                }
                if (it.fileName().startsWith(prefix) && QLibrary::isLibrary(it.fileName())) {
#else
                if (QLibrary::isLibrary(it.fileName())) {
#endif
                    callback(it.fileInfo().absoluteFilePath());
                }
            }
        }
    }

    struct StaticPluginLoadResult {
        QString fileName;
        QJsonObject metaData;
    };
    // This is only relevant in the findPlugins context and thus internal API.
    // If one has a static plugin from QPluginLoader::staticPlugins and does not want it to have metadata, using KPluginMetaData makes no sense
    StaticPluginLoadResult loadStaticPlugin(QStaticPlugin plugin, KPluginMetaData::KPluginMetaDataOption option)
    {
        staticPlugin = plugin;
        auto metaDataObject = plugin.metaData().value(QLatin1String("MetaData")).toObject();
        m_option = option;
        auto names = plugin.metaData().value(QLatin1String("X-KDE-FileName")).toVariant().toStringList();
        QString fileName;
        if (!names.isEmpty()) {
            fileName = names.constFirst();
        }
        return {fileName, metaDataObject};
    }
    static void getPluginLoaderForPath(QPluginLoader &loader, const QString &path)
    {
        if (path.startsWith(QLatin1Char('/'))) { // Absolute path, use as it is
            loader.setFileName(path);
        } else {
            loader.setFileName(QCoreApplication::applicationDirPath() + QLatin1Char('/') + path);
            if (loader.fileName().isEmpty()) {
                loader.setFileName(path);
            }
        }
    }
};

KPluginMetaData::KPluginMetaData()
    : KPluginMetaData({}, {}, {})
{
}

KPluginMetaData::KPluginMetaData(const KPluginMetaData &other)
    : m_metaData(other.m_metaData)
    , m_fileName(other.fileName())
    , d(other.d)
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
    : KPluginMetaData(file, DoNotAllowEmptyMetaData)
{
}

KPluginMetaData::KPluginMetaData(const QString &file, KPluginMetaDataOption option)
    : d(new KPluginMetaDataPrivate)
{
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 92)
    if (file.endsWith(QLatin1String(".desktop"))) {
        Q_ASSERT_X(option == DoNotAllowEmptyMetaData, Q_FUNC_INFO, "The AllowEmptyMetaData flag is only allowed for binary plugins");
        qCDebug(KCOREADDONS_DEBUG)
            << "Using the KPluginMetaData(const QString &file) constructor for desktop files is deprcated, use KPluginMetaData::fromDesktopFile instead";
        loadFromDesktopFile(file, QStringList());
    } else if (file.endsWith(QLatin1String(".json"))) {
        Q_ASSERT_X(option == DoNotAllowEmptyMetaData, Q_FUNC_INFO, "The AllowEmptyMetaData flag is only allowed for binary plugins");
        qCDebug(KCOREADDONS_DEBUG)
            << "Using the KPluginMetaData(const QString &file) constructor for json files is deprcated, use KPluginMetaData::fromJsonFile instead";
        loadFromJsonFile(file);
    } else {
#endif
        d->m_option = option;
        QPluginLoader loader;
        KPluginMetaDataPrivate::getPluginLoaderForPath(loader, file);
        d->m_requestedFileName = file;
        m_fileName = QFileInfo(loader.fileName()).absoluteFilePath();
        const auto qtMetaData = loader.metaData();
        if (!qtMetaData.isEmpty()) {
            m_metaData = qtMetaData.value(QStringLiteral("MetaData")).toObject();
            d->metaDataFileName = m_fileName;
            if (m_metaData.isEmpty() && option == DoNotAllowEmptyMetaData) {
                qCDebug(KCOREADDONS_DEBUG) << "plugin metadata in" << file << "does not have a valid 'MetaData' object";
            }
        } else {
            qCDebug(KCOREADDONS_DEBUG) << "no metadata found in" << file << loader.errorString();
        }
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 92)
    }
#endif
}

KPluginMetaData::KPluginMetaData(const QPluginLoader &loader)
    : KPluginMetaData(loader.metaData().value(QStringLiteral("MetaData")).toObject(), QFileInfo(loader.fileName()).absoluteFilePath())
{
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 86)
KPluginMetaData::KPluginMetaData(const KPluginLoader &loader)
    : KPluginMetaData(loader.metaData().value(QStringLiteral("MetaData")).toObject(), QFileInfo(loader.fileName()).absoluteFilePath())
{
}
#endif

KPluginMetaData::KPluginMetaData(const QJsonObject &metaData, const QString &file)
    : KPluginMetaData(metaData, file, QString())
{
}

KPluginMetaData::KPluginMetaData(const QJsonObject &metaData, const QString &pluginFile, const QString &metaDataFile)
    : m_metaData(metaData)
    , m_fileName(pluginFile)
    , d(new KPluginMetaDataPrivate)
{
    d->metaDataFileName = metaDataFile;
}

KPluginMetaData::KPluginMetaData(QStaticPlugin plugin, const QJsonObject &metaData)
    : d(new KPluginMetaDataPrivate)
{
    const auto result = d->loadStaticPlugin(plugin, DoNotAllowEmptyMetaData);
    m_fileName = result.fileName;
    m_metaData = result.metaData.isEmpty() ? metaData : result.metaData;
}

KPluginMetaData KPluginMetaData::findPluginById(const QString &directory, const QString &pluginId)

{
    QPluginLoader loader;
    KPluginMetaDataPrivate::getPluginLoaderForPath(loader, directory + QLatin1Char('/') + pluginId);
    if (loader.load()) {
        // Load the JSON metadata and make sure the pluginId matches
        KPluginMetaData metaData(loader.metaData().value(QLatin1String("MetaData")).toObject(), loader.fileName());
        if (metaData.isValid() && metaData.pluginId() == pluginId) {
            return metaData;
        }
    }

    // TODO KF6 remove, this is fallback logic if the pluginId is not the same as the file name
    auto filter = [&pluginId](const KPluginMetaData &md) -> bool {
        return md.pluginId() == pluginId;
    };
    const QVector<KPluginMetaData> metaDataVector = KPluginMetaData::findPlugins(directory, filter);
    if (!metaDataVector.isEmpty()) {
        return metaDataVector.first();
    }

    return KPluginMetaData{};
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 92)
KPluginMetaData KPluginMetaData::fromDesktopFile(const QString &file, const QStringList &serviceTypes)
{
    KPluginMetaData result;
    result.loadFromDesktopFile(file, serviceTypes);
    return result;
}

void KPluginMetaData::loadFromDesktopFile(const QString &file, const QStringList &serviceTypes)
{
    QString libraryPath;
    if (!DesktopFileParser::convert(file, serviceTypes, m_metaData, &libraryPath, QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation))) {
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
#endif

void KPluginMetaData::loadFromJsonFile(const QString &file)
{
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
    QString abspath = QFileInfo(file).absoluteFilePath();
    m_fileName = abspath;
    d->metaDataFileName = abspath;
}

KPluginMetaData KPluginMetaData::fromJsonFile(const QString &file)
{
    KPluginMetaData result;
    result.loadFromJsonFile(file);
    return result;
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
    return d && !d->metaDataFileName.isEmpty() ? d->metaDataFileName : m_fileName;
}

QVector<KPluginMetaData> KPluginMetaData::findPlugins(const QString &directory, std::function<bool(const KPluginMetaData &)> filter)
{
    return findPlugins(directory, filter, KPluginMetaData::DoNotAllowEmptyMetaData);
}

QVector<KPluginMetaData>
KPluginMetaData::findPlugins(const QString &directory, std::function<bool(const KPluginMetaData &)> filter, KPluginMetaDataOption option)
{
    QVector<KPluginMetaData> ret;
    const auto staticPlugins = KStaticPluginHelpers::staticPlugins(directory);
    for (QStaticPlugin p : staticPlugins) {
        KPluginMetaData metaData;
        const auto loadingResult = metaData.d->loadStaticPlugin(p, option);
        metaData.m_fileName = loadingResult.fileName;
        metaData.m_metaData = loadingResult.metaData;
        if (metaData.isValid()) {
            if (!filter || filter(metaData)) {
                ret << metaData;
            }
        }
    }
    QSet<QString> addedPluginIds;
    KPluginMetaDataPrivate::forEachPlugin(directory, [&](const QString &pluginPath) {
        KPluginMetaData metadata(pluginPath, option);
        if (!metadata.isValid()) {
            qCDebug(KCOREADDONS_DEBUG) << pluginPath << "does not contain valid JSON metadata";
            return;
        }
        if (addedPluginIds.contains(metadata.pluginId())) {
            return;
        }
        if (filter && !filter(metadata)) {
            return;
        }
        addedPluginIds << metadata.pluginId();
        ret.append(metadata);
    });
    return ret;
}

bool KPluginMetaData::isValid() const
{
    // it can be valid even if m_fileName is empty (as long as the plugin id is set)
    return !pluginId().isEmpty() && (!m_metaData.isEmpty() || d->m_option == AllowEmptyMetaData);
}

bool KPluginMetaData::isHidden() const
{
    return rootObject()[QStringLiteral("Hidden")].toBool();
}

QJsonObject KPluginMetaData::rootObject() const
{
    return m_metaData[QStringLiteral("KPlugin")].toObject();
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 88)
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
        qCWarning(KCOREADDONS_DEBUG) << "Expected JSON property" << key
                                     << "to be a string list."
                                        " Treating it as a list with a single entry:"
                                     << asString << id.toLatin1().constData();
        return QStringList(asString);
    }
}

QJsonValue KPluginMetaData::readTranslatedValue(const QJsonObject &jo, const QString &key, const QJsonValue &defaultValue)
{
    return KJsonUtils::readTranslatedValue(jo, key, defaultValue);
}

QString KPluginMetaData::readTranslatedString(const QJsonObject &jo, const QString &key, const QString &defaultValue)
{
    return readTranslatedValue(jo, key, defaultValue).toString(defaultValue);
}
#endif

static inline void addPersonFromJson(const QJsonObject &obj, QList<KAboutPerson> *out)
{
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
        const QJsonArray peopleArray = people.toArray();
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
    return KJsonUtils::readTranslatedString(rootObject(), QStringLiteral("Description"));
}

QString KPluginMetaData::iconName() const
{
    return rootObject()[QStringLiteral("Icon")].toString();
}

QString KPluginMetaData::license() const
{
    return rootObject()[QStringLiteral("License")].toString();
}

QString KPluginMetaData::licenseText() const
{
    return KAboutLicense::byKeyword(license()).text();
}

QString KPluginMetaData::name() const
{
    return KJsonUtils::readTranslatedString(rootObject(), QStringLiteral("Name"));
}

QString KPluginMetaData::copyrightText() const
{
    return KJsonUtils::readTranslatedString(rootObject(), QStringLiteral("Copyright"));
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 87)
QString KPluginMetaData::extraInformation() const
{
    return KJsonUtils::readTranslatedString(rootObject(), QStringLiteral("ExtraInformation"));
}
#endif

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
    return QFileInfo(m_fileName).completeBaseName();
}

QString KPluginMetaData::version() const
{
    return rootObject()[QStringLiteral("Version")].toString();
}

QString KPluginMetaData::website() const
{
    return rootObject()[QStringLiteral("Website")].toString();
}

QString KPluginMetaData::bugReportUrl() const
{
    return rootObject()[QStringLiteral("BugReportUrl")].toString();
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 79)
QStringList KPluginMetaData::dependencies() const
{
    return readStringList(rootObject(), QStringLiteral("Dependencies"));
}
#endif

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 89)
QStringList KPluginMetaData::serviceTypes() const
{
    return rootObject().value(QStringLiteral("ServiceTypes")).toVariant().toStringList();
}
#endif

QStringList KPluginMetaData::mimeTypes() const
{
    return rootObject().value(QStringLiteral("MimeTypes")).toVariant().toStringList();
}

bool KPluginMetaData::supportsMimeType(const QString &mimeType) const
{
    // Check for exact matches first. This can delay parsing the full MIME
    // database until later and noticeably speed up application startup on
    // slower systems.
    const QStringList mimes = mimeTypes();
    if (mimes.contains(mimeType)) {
        return true;
    }

    // Now check for MIME type inheritance to find non-exact matches:
    QMimeDatabase db;
    const QMimeType mime = db.mimeTypeForName(mimeType);
    if (!mime.isValid()) {
        return false;
    }

    auto inherits = [&](const QString &supportedMimeName) {
        return mime.inherits(supportedMimeName);
    };
    return std::find_if(mimes.begin(), mimes.end(), inherits) != mimes.end();
}

QStringList KPluginMetaData::formFactors() const
{
    return rootObject().value(QStringLiteral("FormFactors")).toVariant().toStringList();
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

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 104)
int KPluginMetaData::initialPreference() const
{
    return rootObject()[QStringLiteral("InitialPreference")].toInt();
}
#endif

QString KPluginMetaData::value(const QString &key, const QString &defaultValue) const
{
    const QJsonValue value = m_metaData.value(key);
    if (value.isString()) {
        return value.toString(defaultValue);
    } else if (value.isArray()) {
        qCWarning(KCOREADDONS_DEBUG) << "Expected JSON property" << key
                                     << "to be a single string."
                                        " but it is a stringlist";
        const QStringList list = value.toVariant().toStringList();
        if (list.isEmpty()) {
            return defaultValue;
        }
        return list.join(QChar::fromLatin1(','));
    } else if (value.isBool()) {
        qCWarning(KCOREADDONS_DEBUG) << "Expected JSON property" << key
                                     << "to be a single string."
                                        " but it is a bool";
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    }
    return defaultValue;
}

bool KPluginMetaData::value(const QString &key, bool defaultValue) const
{
    const QJsonValue value = m_metaData.value(key);
    if (value.isBool()) {
        return value.toBool();
    } else if (value.isString()) {
        return value.toString() == QLatin1String("true");
    } else {
        return defaultValue;
    }
}

int KPluginMetaData::value(const QString &key, int defaultValue) const
{
    const QJsonValue value = m_metaData.value(key);
    if (value.isDouble()) {
        return value.toInt();
    } else if (value.isString()) {
        const QString intString = value.toString();
        bool ok;
        int convertedIntValue = intString.toInt(&ok);
        if (ok) {
            return convertedIntValue;
        } else {
            qCWarning(KCOREADDONS_DEBUG) << "Expected" << key << "to be an int, instead" << intString << "was specified in the json metadata" << m_fileName;
            return defaultValue;
        }
    } else {
        return defaultValue;
    }
}
QStringList KPluginMetaData::value(const QString &key, const QStringList &defaultValue) const
{
    const QJsonValue value = m_metaData.value(key);
    if (value.isUndefined() || value.isNull()) {
        return defaultValue;
    } else if (value.isObject()) {
        qCWarning(KCOREADDONS_DEBUG) << "Expected JSON property" << key << "to be a string list, instead an object was specified in the json metadata"
                                     << m_fileName;
        return defaultValue;
    } else if (value.isArray()) {
        return value.toVariant().toStringList();
    } else {
        const QString asString = value.isString() ? value.toString() : value.toVariant().toString();
        if (asString.isEmpty()) {
            return defaultValue;
        }
        qCDebug(KCOREADDONS_DEBUG) << "Expected JSON property" << key << "to be a string list in the json metadata" << m_fileName
                                   << "Treating it as a list with a single entry:" << asString;
        return QStringList(asString);
    }
}

bool KPluginMetaData::operator==(const KPluginMetaData &other) const
{
    return m_fileName == other.m_fileName && m_metaData == other.m_metaData;
}

#if KCOREADDONS_ENABLE_DEPRECATED_SINCE(5, 86)
QObject *KPluginMetaData::instantiate() const
{
    QPluginLoader loader(m_fileName);
    const auto ret = loader.instance();
    if (!ret) {
        qCWarning(KCOREADDONS_DEBUG) << "Could not create plugin" << name() << "error:" << loader.errorString();
    }
    return ret;
}
#endif

template<class T>
QVariantList listToVariant(const QList<T> &values)
{
    QVariantList ret;
    ret.reserve(values.count());
    std::transform(values.cbegin(), values.cend(), std::back_inserter(ret), [](const T &license) {
        return QVariant::fromValue(license);
    });
    return ret;
}

QVariantList KPluginMetaData::authorsVariant() const
{
    return listToVariant(authors());
}

QVariantList KPluginMetaData::translatorsVariant() const
{
    return listToVariant(translators());
}

QVariantList KPluginMetaData::otherContributorsVariant() const
{
    return listToVariant(otherContributors());
}
bool KPluginMetaData::isStaticPlugin() const
{
    return d && d->staticPlugin.has_value();
}

QString KPluginMetaData::requestedFileName() const
{
    return d ? d->m_requestedFileName : QString();
}

QStaticPlugin KPluginMetaData::staticPlugin() const
{
    Q_ASSERT(d);
    Q_ASSERT(d->staticPlugin.has_value());
    return d->staticPlugin.value();
}
