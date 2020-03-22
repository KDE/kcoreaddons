/*
    SPDX-FileCopyrightText: 2013-2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/


#include "desktopfileparser_p.h"

#include <QCache>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QMutex>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

// in the desktoptojson binary enable debug messages by default, in the library only warning messages
#ifdef BUILDING_DESKTOPTOJSON_TOOL
Q_LOGGING_CATEGORY(DESKTOPPARSER, "kf5.kcoreaddons.desktopparser", QtDebugMsg)
#else
Q_LOGGING_CATEGORY(DESKTOPPARSER, "kf5.kcoreaddons.desktopparser", QtWarningMsg)
#endif


#ifdef BUILDING_DESKTOPTOJSON_TOOL
// use if not else to prevent wrong scoping
#define DESKTOPTOJSON_VERBOSE_DEBUG if (!DesktopFileParser::s_verbose) {} else qCDebug(DESKTOPPARSER)
#define DESKTOPTOJSON_VERBOSE_WARNING if (!DesktopFileParser::s_verbose) {} else qCWarning(DESKTOPPARSER)
#else
#define DESKTOPTOJSON_VERBOSE_DEBUG QT_NO_QDEBUG_MACRO()
#define DESKTOPTOJSON_VERBOSE_WARNING QT_NO_QDEBUG_MACRO()
#endif


using namespace DesktopFileParser;

// This code was taken from KConfigGroupPrivate::deserializeList
QStringList DesktopFileParser::deserializeList(const QString &data, char separator)
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
        } else if (data[p].unicode() == separator) {
            value.append(val);
            if (p == data.length() - 1) {
                // don't add an empty entry to the end if the last character is a separator
                return value;
            }
            val.clear();
            val.reserve(data.size() - p);
        } else {
            val += data[p];
        }
    }
    value.append(val);
    return value;
}

QByteArray DesktopFileParser::escapeValue(const QByteArray &input)
{
    const int start = input.indexOf('\\');
    if (start < 0) {
        return input;
    }

    // we could do this in place, but this code is simpler
    // this tool is probably only transitional, so no need to optimize
    QByteArray result;
    result.reserve(input.size());
    result.append(input.data(), start);
    for (int i = start; i < input.length(); ++i) {
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

struct CustomPropertyDefinition {
    // default ctor needed for QVector
    CustomPropertyDefinition() : type(QVariant::String) {}
    CustomPropertyDefinition(const QByteArray &key, QVariant::Type type)
        : key(key) , type(type) {}
    QJsonValue fromString(const QString &str) const
    {
        switch (type) {
        case QVariant::String:
            return str;
        case QVariant::StringList:
            return QJsonArray::fromStringList(deserializeList(str));
        case QVariant::Int: {
            bool ok = false;
            int result = str.toInt(&ok);
            if (!ok) {
                qCWarning(DESKTOPPARSER) << "Invalid integer value for key" << key << "-" << str;
                return QJsonValue();
            }
            return QJsonValue(result);
        }
        case QVariant::Double: {
            bool ok = false;
            double result = str.toDouble(&ok);
            if (!ok) {
                qCWarning(DESKTOPPARSER) << "Invalid double value for key" << key << "-" << str;
                return QJsonValue();
            }
            return QJsonValue(result);
        }
        case QVariant::Bool: {
            bool result = str.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
            if (!result && str.compare(QLatin1String("false"), Qt::CaseInsensitive) != 0) {
                qCWarning(DESKTOPPARSER) << "Invalid boolean value for key" << key << "-" << str;
                return QJsonValue();
            }
            return QJsonValue(result);
        }
        default:
            // This was checked when parsing the file, no other QVariant::Type values are possible
            Q_UNREACHABLE();
        }
    }
    QByteArray key;
    QVariant::Type type;
};

namespace {

bool readUntilDesktopEntryGroup(QFile &file, const QString &path, int &lineNr)
{
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(DESKTOPPARSER) << "Error: Failed to open " << path;
        return false;
    }
    // we only convert data inside the [Desktop Entry] group
    while (!file.atEnd()) {
        const QByteArray line = file.readLine().trimmed();
        lineNr++;
        if (line == "[Desktop Entry]") {
            return true;
        }
    }
    qCWarning(DESKTOPPARSER) << "Error: Could not find [Desktop Entry] group in " << path;
    return false;
}


QByteArray readTypeEntryForCurrentGroup(QFile &df, QByteArray *nextGroup, QByteArray *pName)
{
    QByteArray group = *nextGroup;
    QByteArray type;
    if (group.isEmpty()) {
        qCWarning(DESKTOPPARSER, "Read empty .desktop file group name! Invalid file?");
    }
    while (!df.atEnd()) {
        QByteArray line = df.readLine().trimmed();
        // skip empty lines and comments
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        if (line.startsWith('[')) {
            if (!line.endsWith(']')) {
                qCWarning(DESKTOPPARSER) << "Illegal .desktop group definition (does not end with ']'):" << line;
            }
            QByteArray name = line.mid(1, line.lastIndexOf(']') - 1).trimmed();
            // we have reached the next group -> return current group and Type= value
            *nextGroup = name;
            break;
        }

        const static QRegularExpression typeEntryRegex(
                QStringLiteral("^Type\\s*=\\s*(.*)$"));
        const auto match = typeEntryRegex.match(QString::fromUtf8(line));
        if (match.hasMatch()) {
            type = match.captured(1).toUtf8();
        } else if (pName) {
            const static QRegularExpression nameEntryRegex(
                    QStringLiteral("^X-KDE-ServiceType\\s*=\\s*(.*)$"));
            const auto nameMatch = nameEntryRegex.match(QString::fromUtf8(line));
            if (nameMatch.hasMatch()) {
                *pName = nameMatch.captured(1).toUtf8();
            }
        }
    }
    return type;
}

bool tokenizeKeyValue(QFile &df, const QString &src, QByteArray &key, QString &value, int &lineNr)
{
    const QByteArray line = df.readLine().trimmed();
    lineNr++;
    if (line.isEmpty()) {
        DESKTOPTOJSON_VERBOSE_DEBUG << "Line " << lineNr << ": empty";
        return true;
    }
    if (line.startsWith('#')) {
        DESKTOPTOJSON_VERBOSE_DEBUG << "Line " << lineNr << ": comment";
        return true; // skip comments
    }
    if (line.startsWith('[')) {
        // start of new group -> doesn't interest us anymore
        DESKTOPTOJSON_VERBOSE_DEBUG << "Line " << lineNr << ": start of new group " << line;
        return false;
    }
    // must have form key=value now
    const int equalsIndex = line.indexOf('=');
    if (equalsIndex == -1) {
        qCWarning(DESKTOPPARSER).nospace() << qPrintable(src) << ':' << lineNr << ": Line is neither comment nor group "
            "and doesn't contain an '=' character: \"" << line.constData() << '\"';
        return true;
    }
    // trim key and value to remove spaces around the '=' char
    key = line.mid(0, equalsIndex).trimmed();
    if (key.isEmpty()) {
        qCWarning(DESKTOPPARSER).nospace() << qPrintable(src) << ':' << lineNr << ": Key name is missing: \"" << line.constData() << '\"';
        return true;
    }

    const QByteArray valueRaw = line.mid(equalsIndex + 1).trimmed();
    const QByteArray valueEscaped = escapeValue(valueRaw);
    value = QString::fromUtf8(valueEscaped);

#ifdef BUILDING_DESKTOPTOJSON_TOOL
    DESKTOPTOJSON_VERBOSE_DEBUG.nospace() << "Line " << lineNr << ": key=" << key << ", value=" << value;
    if (valueEscaped != valueRaw) {
        DESKTOPTOJSON_VERBOSE_DEBUG << "Line " << lineNr << " contained escape sequences";
    }
#endif

    return true;
}

static QString locateRelativeServiceType(const QString &relPath)
{
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                  QStringLiteral("kservicetypes5/") + relPath);
}

static ServiceTypeDefinition* parseServiceTypesFile(const QString &inputPath)
{
    int lineNr = 0;
    QString path = inputPath;
    if (QDir::isRelativePath(path)) {
        path = locateRelativeServiceType(path);
        QString rcPath;
        if (path.isEmpty()) {
            rcPath = QLatin1String(":/kservicetypes5/") + inputPath;
            if (QFileInfo::exists(rcPath)) {
                path = rcPath;
            }
        }
        if (path.isEmpty()) {
            qCWarning(DESKTOPPARSER).nospace() << "Could not locate service type file kservicetypes5/" << qPrintable(inputPath) << ", tried " << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation) << " and " << rcPath;
            return nullptr;
        }
    }
    QFile df(path);
    if (!df.exists()) {
        qCCritical(DESKTOPPARSER) << "Service type file" << path << "does not exist";
        return nullptr;
    }
    if (!readUntilDesktopEntryGroup(df, path, lineNr)) {
        return nullptr;
    }
    ServiceTypeDefinition result;
    // TODO: passing nextGroup by pointer is inefficient as it will make deep copies every time
    // Not exactly performance critical code though so low priority
    QByteArray nextGroup = "Desktop Entry";
    // Type must be ServiceType now
    QByteArray typeStr = readTypeEntryForCurrentGroup(df, &nextGroup, &result.m_serviceTypeName);
    if (typeStr != QByteArrayLiteral("ServiceType")) {
        qCWarning(DESKTOPPARSER) << path << "is not a valid service type: Type entry should be 'ServiceType', got"
            << typeStr << "instead.";
        return nullptr;
    }
    while (!df.atEnd()) {
        QByteArray currentGroup = nextGroup;
        typeStr = readTypeEntryForCurrentGroup(df, &nextGroup, nullptr);
        if (!currentGroup.startsWith(QByteArrayLiteral("PropertyDef::"))) {
            qCWarning(DESKTOPPARSER) << "Skipping invalid group" << currentGroup << "in service type" << path;
            continue;
        }
        if (typeStr.isEmpty()) {
            qCWarning(DESKTOPPARSER) << "Could not find Type= key in group" << currentGroup;
            continue;
        }
        QByteArray propertyName = currentGroup.mid(qstrlen("PropertyDef::"));
        QVariant::Type type = QVariant::nameToType(typeStr.constData());
        switch (type) {
            case QVariant::String:
            case QVariant::StringList:
            case QVariant::Int:
            case QVariant::Double:
            case QVariant::Bool:
                qCDebug(DESKTOPPARSER) << "Found property definition" << propertyName << "with type" << typeStr;
                result.m_propertyDefs.push_back(CustomPropertyDefinition(propertyName, type));
                break;
            case QVariant::Invalid:
                qCWarning(DESKTOPPARSER) << "Property type" << typeStr << "is not a known QVariant type."
                        " Found while parsing property definition for" << propertyName << "in" << path;
                break;
            default:
                qCWarning(DESKTOPPARSER) << "Unsupported property type" << typeStr << "for property" << propertyName
                        << "found in" << path << "\nOnly QString, QStringList, int, double and bool are supported.";
        }
    }
    return new ServiceTypeDefinition(result);
}

// a lazy map of service type definitions
typedef QCache<QString /*path*/, ServiceTypeDefinition> ServiceTypesHash;
Q_GLOBAL_STATIC(ServiceTypesHash, s_serviceTypes)
// access must be guarded by serviceTypesMutex as this code could be executed by multiple threads
QBasicMutex s_serviceTypesMutex;
} // end of anonymous namespace


ServiceTypeDefinitions ServiceTypeDefinitions::fromFiles(const QStringList &paths)
{
    ServiceTypeDefinitions ret;
    ret.m_definitions.reserve(paths.size());
    // as we might modify the cache we need to acquire a mutex here
    for (const QString &serviceTypePath : paths) {
        bool added = ret.addFile(serviceTypePath);
        if (!added) {
#ifdef BUILDING_DESKTOPTOJSON_TOOL
            exit(1); // this is a fatal error when using kcoreaddons_desktop_to_json()
#endif
        }
    }
    return ret;
}

bool ServiceTypeDefinitions::addFile(const QString& path)
{
    QMutexLocker lock(&s_serviceTypesMutex);
    ServiceTypeDefinition* def = s_serviceTypes->object(path);

    if (def) {
        // in cache but we still must make our own copy
        m_definitions << *def;
    } else {
        // not found in cache -> we need to parse the file
        qCDebug(DESKTOPPARSER) << "About to parse service type file" << path;
        def = parseServiceTypesFile(path);
        if (!def) {
            return false;
        }

        m_definitions << *def; // This must *precede* insert call, insert might delete
        s_serviceTypes->insert(path, def);
    }
    return true;
}

QJsonValue ServiceTypeDefinitions::parseValue(const QByteArray &key, const QString &value) const
{
    // check whether the key has a special type associated with it
    for (const auto &def : m_definitions) {
        for (const CustomPropertyDefinition &propertyDef : def.m_propertyDefs) {
            if (propertyDef.key == key) {
                return propertyDef.fromString(value);
            }
        }
    }
    qCDebug(DESKTOPPARSER) << "Unknown property type for key" << key << "-> falling back to string";
    return QJsonValue(value);
}

bool ServiceTypeDefinitions::hasServiceType(const QByteArray &serviceTypeName) const
{
    const auto it = std::find_if(m_definitions.begin(), m_definitions.end(), [&serviceTypeName](const ServiceTypeDefinition &def) {
            return def.m_serviceTypeName == serviceTypeName;
    });
    return it != m_definitions.end();
}

void DesktopFileParser::convertToJson(const QByteArray &key, ServiceTypeDefinitions &serviceTypes, const QString &value,
                                      QJsonObject &json, QJsonObject &kplugin, int lineNr)
{
    /* The following keys are recognized (and added to a "KPlugin" object):

        Icon=mypluginicon
        Type=Service
        ServiceTypes=KPluginInfo
        MimeType=text/plain;image/png

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
        X-KDE-FormFactors=desktop
    */
    if (key == QByteArrayLiteral("Icon")) {
        kplugin[QStringLiteral("Icon")] = value;
    } else if (key == QByteArrayLiteral("X-KDE-PluginInfo-Name")) {
        kplugin[QStringLiteral("Id")] = value;
    } else if (key == QByteArrayLiteral("X-KDE-PluginInfo-Category")) {
        kplugin[QStringLiteral("Category")] = value;
    } else if (key == QByteArrayLiteral("X-KDE-PluginInfo-License")) {
        kplugin[QStringLiteral("License")] = value;
    } else if (key == QByteArrayLiteral("X-KDE-PluginInfo-Version")) {
        kplugin[QStringLiteral("Version")] = value;
    } else if (key == QByteArrayLiteral("X-KDE-PluginInfo-Website")) {
        kplugin[QStringLiteral("Website")] = value;
    } else if (key == QByteArrayLiteral("X-KDE-PluginInfo-Depends")) {
        kplugin[QStringLiteral("Dependencies")] = QJsonArray::fromStringList(deserializeList(value));
    } else if (key == QByteArrayLiteral("X-KDE-ServiceTypes") || key == QByteArrayLiteral("ServiceTypes")) {
        //NOTE: "X-KDE-ServiceTypes" and "ServiceTypes" were already managed in the first parse step, so this second one is almost a noop
        const auto services = deserializeList(value);
        kplugin[QStringLiteral("ServiceTypes")] = QJsonArray::fromStringList(services);
    } else if (key == QByteArrayLiteral("MimeType")) {
        // MimeType is a XDG string list and not a KConfig list so we need to use ';' as the separator
        kplugin[QStringLiteral("MimeTypes")] = QJsonArray::fromStringList(deserializeList(value, ';'));
        // make sure that applications using kcoreaddons_desktop_to_json() that depend on reading
        // the MimeType property still work (see https://git.reviewboard.kde.org/r/125527/)
        json[QStringLiteral("MimeType")] = value; // TODO KF6 remove this compatibility code
    } else if (key == QByteArrayLiteral("X-KDE-FormFactors")) {
        kplugin[QStringLiteral("FormFactors")] = QJsonArray::fromStringList(deserializeList(value));
    } else if (key == QByteArrayLiteral("X-KDE-PluginInfo-EnabledByDefault")) {
        bool boolValue = false;
        // should only be lower case, but be tolerant here
        if (value.toLower() == QLatin1String("true")) {
            boolValue = true;
        } else {
            if (value.toLower() != QLatin1String("false")) {
                qCWarning(DESKTOPPARSER).nospace() << "Expected boolean value for key \"" << key
                    << "\" at line " << lineNr << "but got \"" << value << "\" instead.";
            }
        }
        kplugin[QStringLiteral("EnabledByDefault")] = boolValue;
    } else if (key == QByteArrayLiteral("X-KDE-PluginInfo-Author")) {
        QJsonObject authorsObject = kplugin.value(QStringLiteral("Authors")).toArray().at(0).toObject();
        // if the authors object doesn't exist yet this will create it
        authorsObject[QStringLiteral("Name")] = value;
        QJsonArray array;
        array.append(authorsObject);
        kplugin[QStringLiteral("Authors")] = array;
    } else if (key == QByteArrayLiteral("X-KDE-PluginInfo-Email")) {
        QJsonObject authorsObject = kplugin.value(QStringLiteral("Authors")).toArray().at(0).toObject();
        // if the authors object doesn't exist yet this will create it
        authorsObject[QStringLiteral("Email")] = value;
        QJsonArray array;
        array.append(authorsObject);
        kplugin[QStringLiteral("Authors")] = array;
    } else if (key == QByteArrayLiteral("Name") || key.startsWith(QByteArrayLiteral("Name["))) {
        // TODO: also handle GenericName? does that make any sense, or is X-KDE-PluginInfo-Category enough?
        kplugin[QString::fromUtf8(key)] = value;
    } else if (key == QByteArrayLiteral("Comment")) {
        kplugin[QStringLiteral("Description")] = value;
    } else if (key.startsWith(QByteArrayLiteral("Comment["))) {
        kplugin[QStringLiteral("Description") + QString::fromUtf8(key.mid(qstrlen("Comment")))] = value;
    } else if (key == QByteArrayLiteral("InitialPreference")) {
        kplugin[QStringLiteral("InitialPreference")] = value.toInt();
    } else if (key == QByteArrayLiteral("Hidden")) {
        DESKTOPTOJSON_VERBOSE_WARNING << "Hidden= key found in desktop file, this makes no sense"
                " with metadata inside the plugin.";
        kplugin[QString::fromUtf8(key)] = (value.toLower() == QLatin1String("true"));
    } else if (key == QByteArrayLiteral("Exec") || key == QByteArrayLiteral("Type")
            || key == QByteArrayLiteral("X-KDE-Library") || key == QByteArrayLiteral("Encoding")) {
        // Exec= doesn't make sense here, however some .desktop files (like e.g. in kdevelop) have a dummy value here
        // also the Type=Service entry is no longer needed
        // X-KDE-Library is also not needed since we already have the library to read this metadata
        // Encoding= is also not converted as we always use utf-8 for reading
        DESKTOPTOJSON_VERBOSE_DEBUG << "Not converting key " << key << "=" << value;
    } else {
        // check service type definitions or fall back to QString
        json[QString::fromUtf8(key)] = serviceTypes.parseValue(key, value);
    }
}

bool DesktopFileParser::convert(const QString &src, const QStringList &serviceTypes, QJsonObject &json, QString *libraryPath)
{
    QFile df(src);
    int lineNr = 0;
    ServiceTypeDefinitions serviceTypeDef = ServiceTypeDefinitions::fromFiles(serviceTypes);
    readUntilDesktopEntryGroup(df, src, lineNr);
    DESKTOPTOJSON_VERBOSE_DEBUG << "Found [Desktop Entry] group in line" << lineNr;
    auto startPos = df.pos();

    //parse it a first time to know servicetype
    while (!df.atEnd()) {
        QByteArray key;
        QString value;
        if (!tokenizeKeyValue(df, src, key, value, lineNr)) {
            break;
        }
        // some .desktop files still use the legacy ServiceTypes= key
        if (key == QByteArrayLiteral("X-KDE-ServiceTypes") || key == QByteArrayLiteral("ServiceTypes")) {
            const QString dotDesktop = QStringLiteral(".desktop");
            const QChar slashChar(QLatin1Char('/'));
            const auto serviceList = deserializeList(value);

            for (const auto &service : serviceList) {
                if (!serviceTypeDef.hasServiceType(service.toLatin1())) {
                    // Make up the filename from the service type name. This assumes consistent naming...
                    QString absFileName = locateRelativeServiceType(
                            service.toLower().replace(slashChar, QLatin1Char('-')) + dotDesktop);
                    if (absFileName.isEmpty()) {
                        absFileName = locateRelativeServiceType(
                                service.toLower().remove(slashChar) + dotDesktop);
                    }
                    if (absFileName.isEmpty()) {
                        qCWarning(DESKTOPPARSER) << "Unable to find service type for service" << service << "listed in" << src;
                    } else {
                        serviceTypeDef.addFile(absFileName);
                    }
                }
            }
            break;
        }
    }
    lineNr=0;
    df.seek(startPos);

    QJsonObject kplugin; // the "KPlugin" key of the metadata
    //QJsonObject json;
    while (!df.atEnd()) {
        QByteArray key;
        QString value;
        if (!tokenizeKeyValue(df, src, key, value, lineNr)) {
            break;
        } else if (key.isEmpty()) {
            continue;
        }
#ifdef BUILDING_DESKTOPTOJSON_TOOL
        if (s_compatibilityMode) {
            convertToCompatibilityJson(QString::fromUtf8(key), value, json, lineNr);
        } else {
            convertToJson(key, serviceTypeDef, value, json, kplugin, lineNr);
        }
#else
        convertToJson(key, serviceTypeDef, value, json, kplugin, lineNr);
#endif
        if (libraryPath && key == QByteArrayLiteral("X-KDE-Library")) {
            *libraryPath = value;
        }
    }
    json[QStringLiteral("KPlugin")] = kplugin;
    return true;
}

