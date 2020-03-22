/*
    This file is part of the KDE Libraries

    SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>
    SPDX-FileCopyrightText: 2006 Nicolas GOUTTE <goutte@kde.org>
    SPDX-FileCopyrightText: 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2017 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kaboutdata.h"
#include "kpluginmetadata.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QSharedData>
#include <QList>
#include <QUrl>
#include <QHash>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QJsonObject>
#include <QLoggingCategory>

#include <algorithm>


Q_DECLARE_LOGGING_CATEGORY(KABOUTDATA)
// logging category for this framework, default: log stuff >= warning
Q_LOGGING_CATEGORY(KABOUTDATA, "kf5.kcoreaddons.kaboutdata", QtWarningMsg)


class Q_DECL_HIDDEN KAboutPerson::Private
{
public:
    QString _name;
    QString _task;
    QString _emailAddress;
    QString _webAddress;
    QString _ocsUsername;
};

KAboutPerson::KAboutPerson(const QString &_name,
                           const QString &_task,
                           const QString &_emailAddress,
                           const QString &_webAddress,
                           const QString &_ocsUsername)
    : d(new Private)
{
    d->_name = _name;
    d->_task = _task;
    d->_emailAddress = _emailAddress;
    d->_webAddress = _webAddress;
    d->_ocsUsername = _ocsUsername;
}

KAboutPerson::KAboutPerson(const QString &_name, const QString &_email, bool)
    : d(new Private)
{
    d->_name = _name;
    d->_emailAddress = _email;
}

KAboutPerson::KAboutPerson(const KAboutPerson &other): d(new Private)
{
    *d = *other.d;
}

KAboutPerson::~KAboutPerson()
{
    delete d;
}

QString KAboutPerson::name() const
{
    return d->_name;
}

QString KAboutPerson::task() const
{
    return d->_task;
}

QString KAboutPerson::emailAddress() const
{
    return d->_emailAddress;
}

QString KAboutPerson::webAddress() const
{
    return d->_webAddress;
}

QString KAboutPerson::ocsUsername() const
{
    return d->_ocsUsername;
}

KAboutPerson &KAboutPerson::operator=(const KAboutPerson &other)
{
    *d = *other.d;
    return *this;
}

KAboutPerson KAboutPerson::fromJSON(const QJsonObject &obj)
{
    const QString name = KPluginMetaData::readTranslatedString(obj, QStringLiteral("Name"));
    const QString task = KPluginMetaData::readTranslatedString(obj, QStringLiteral("Task"));
    const QString email = obj[QStringLiteral("Email")].toString();
    const QString website = obj[QStringLiteral("Website")].toString();
    const QString userName = obj[QStringLiteral("UserName")].toString();
    return KAboutPerson(name, task, email, website, userName);
}


class Q_DECL_HIDDEN KAboutLicense::Private : public QSharedData
{
public:
    Private(LicenseKey licenseType,
            VersionRestriction versionRestriction,
            const KAboutData *aboutData);
    Private(const Private &other);

    QString spdxID() const;

    LicenseKey _licenseKey;
    QString _licenseText;
    QString _pathToLicenseTextFile;
    VersionRestriction _versionRestriction;
    // needed for access to the possibly changing copyrightStatement()
    const KAboutData *_aboutData;
};

KAboutLicense::Private::Private(LicenseKey licenseType,
                                VersionRestriction versionRestriction,
                                const KAboutData *aboutData)
    : QSharedData(),
      _licenseKey(licenseType),
      _versionRestriction(versionRestriction),
      _aboutData(aboutData)
{
}

KAboutLicense::Private::Private(const KAboutLicense::Private &other)
    : QSharedData(other),
      _licenseKey(other._licenseKey),
      _licenseText(other._licenseText),
      _pathToLicenseTextFile(other._pathToLicenseTextFile),
      _versionRestriction(other._versionRestriction),
      _aboutData(other._aboutData)
{}

QString KAboutLicense::Private::spdxID() const
{
    switch (_licenseKey) {
    case KAboutLicense::GPL_V2:
        return QStringLiteral("GPL-2.0");
    case KAboutLicense::LGPL_V2:
        return QStringLiteral("LGPL-2.0");
    case KAboutLicense::BSDL:
        return QStringLiteral("BSD-2-Clause");
    case KAboutLicense::Artistic:
        return QStringLiteral("Artistic-1.0");
    case KAboutLicense::QPL_V1_0:
        return QStringLiteral("QPL-1.0");
    case KAboutLicense::GPL_V3:
        return QStringLiteral("GPL-3.0");
    case KAboutLicense::LGPL_V3:
        return QStringLiteral("LGPL-3.0");
    case KAboutLicense::LGPL_V2_1:
        return QStringLiteral("LGPL-2.1");
    case KAboutLicense::Custom:
    case KAboutLicense::File:
    case KAboutLicense::Unknown:
        return QString();
    }
    return QString();
}

KAboutLicense::KAboutLicense()
    : d(new Private(Unknown, {}, nullptr))
{}


KAboutLicense::KAboutLicense(LicenseKey licenseType,
                             VersionRestriction versionRestriction,
                             const KAboutData *aboutData)
    : d(new Private(licenseType, versionRestriction, aboutData))
{

}

KAboutLicense::KAboutLicense(LicenseKey licenseType, const KAboutData *aboutData)
    : d(new Private(licenseType, OnlyThisVersion, aboutData))
{
}

KAboutLicense::KAboutLicense(const KAboutData *aboutData)
    : d(new Private(Unknown, OnlyThisVersion, aboutData))
{
}

KAboutLicense::KAboutLicense(const KAboutLicense &other)
    : d(other.d)
{
}

KAboutLicense::~KAboutLicense()
{}

void KAboutLicense::setLicenseFromPath(const QString &pathToFile)
{
    d->_licenseKey = KAboutLicense::File;
    d->_pathToLicenseTextFile = pathToFile;
}

void KAboutLicense::setLicenseFromText(const QString &licenseText)
{
    d->_licenseKey = KAboutLicense::Custom;
    d->_licenseText = licenseText;
}

QString KAboutLicense::text() const
{
    QString result;

    const QString lineFeed = QStringLiteral("\n\n");

    if (d->_aboutData && !d->_aboutData->copyrightStatement().isEmpty()) {
        result = d->_aboutData->copyrightStatement() + lineFeed;
    }

    bool knownLicense = false;
    QString pathToFile; // rel path if known license
    switch (d->_licenseKey) {
    case KAboutLicense::File:
        pathToFile = d->_pathToLicenseTextFile;
        break;
    case KAboutLicense::GPL_V2:
        knownLicense = true;
        pathToFile = QStringLiteral("GPL_V2");
        break;
    case KAboutLicense::LGPL_V2:
        knownLicense = true;
        pathToFile = QStringLiteral("LGPL_V2");
        break;
    case KAboutLicense::BSDL:
        knownLicense = true;
        pathToFile = QStringLiteral("BSD");
        break;
    case KAboutLicense::Artistic:
        knownLicense = true;
        pathToFile = QStringLiteral("ARTISTIC");
        break;
    case KAboutLicense::QPL_V1_0:
        knownLicense = true;
        pathToFile = QStringLiteral("QPL_V1.0");
        break;
    case KAboutLicense::GPL_V3:
        knownLicense = true;
        pathToFile = QStringLiteral("GPL_V3");
        break;
    case KAboutLicense::LGPL_V3:
        knownLicense = true;
        pathToFile = QStringLiteral("LGPL_V3");
        break;
    case KAboutLicense::LGPL_V2_1:
        knownLicense = true;
        pathToFile = QStringLiteral("LGPL_V21");
        break;
    case KAboutLicense::Custom:
        if (!d->_licenseText.isEmpty()) {
            result = d->_licenseText;
            break;
        }
        Q_FALLTHROUGH();
    // fall through
    default:
        result += QCoreApplication::translate(
                      "KAboutLicense",
                      "No licensing terms for this program have been specified.\n"
                      "Please check the documentation or the source for any\n"
                      "licensing terms.\n");
    }

    if (knownLicense) {
        pathToFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                QLatin1String("kf5/licenses/") + pathToFile);
        result += QCoreApplication::translate(
                      "KAboutLicense",
                      "This program is distributed under the terms of the %1.").arg(name(KAboutLicense::ShortName));
        if (!pathToFile.isEmpty()) {
            result += lineFeed;
        }
    }

    if (!pathToFile.isEmpty()) {
        QFile file(pathToFile);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream str(&file);
            result += str.readAll();
        }
    }

    return result;
}

QString KAboutLicense::spdx() const
{
    // SPDX licenses are comprised of an identifier (e.g. GPL-2.0), an optional + to denote 'or
    // later versions' and optional ' WITH $exception' to denote standardized exceptions from the
    // core license. As we do not offer exceptions we effectively only return GPL-2.0 or GPL-2.0+,
    // this may change in the future. To that end the documentation makes no assertations about the
    // actual content of the SPDX license expression we return.
    // Expressions can in theory also contain AND, OR and () to build constructs involving more than
    // one license. As this is outside the scope of a single license object we'll ignore this here
    // for now.
    // The expecation is that the return value is only run through spec-compliant parsers, so this
    // can potentially be changed.

    auto id = d->spdxID();
    if (id.isNull()) { // Guard against potential future changes which would allow 'Foo+' as input.
        return id;
    }
    return d->_versionRestriction == OrLaterVersions ? id.append(QLatin1Char('+')) : id;
}

QString KAboutLicense::name(KAboutLicense::NameFormat formatName) const
{
    QString licenseShort;
    QString licenseFull;

    switch (d->_licenseKey) {
    case KAboutLicense::GPL_V2:
        licenseShort = QCoreApplication::translate("KAboutLicense", "GPL v2", "@item license (short name)");
        licenseFull = QCoreApplication::translate("KAboutLicense", "GNU General Public License Version 2", "@item license");
        break;
    case KAboutLicense::LGPL_V2:
        licenseShort = QCoreApplication::translate("KAboutLicense", "LGPL v2", "@item license (short name)");
        licenseFull = QCoreApplication::translate("KAboutLicense", "GNU Lesser General Public License Version 2", "@item license");
        break;
    case KAboutLicense::BSDL:
        licenseShort = QCoreApplication::translate("KAboutLicense", "BSD License", "@item license (short name)");
        licenseFull = QCoreApplication::translate("KAboutLicense", "BSD License", "@item license");
        break;
    case KAboutLicense::Artistic:
        licenseShort = QCoreApplication::translate("KAboutLicense", "Artistic License", "@item license (short name)");
        licenseFull = QCoreApplication::translate("KAboutLicense", "Artistic License", "@item license");
        break;
    case KAboutLicense::QPL_V1_0:
        licenseShort = QCoreApplication::translate("KAboutLicense", "QPL v1.0", "@item license (short name)");
        licenseFull = QCoreApplication::translate("KAboutLicense", "Q Public License", "@item license");
        break;
    case KAboutLicense::GPL_V3:
        licenseShort = QCoreApplication::translate("KAboutLicense", "GPL v3", "@item license (short name)");
        licenseFull = QCoreApplication::translate("KAboutLicense", "GNU General Public License Version 3", "@item license");
        break;
    case KAboutLicense::LGPL_V3:
        licenseShort = QCoreApplication::translate("KAboutLicense", "LGPL v3", "@item license (short name)");
        licenseFull = QCoreApplication::translate("KAboutLicense", "GNU Lesser General Public License Version 3", "@item license");
        break;
    case KAboutLicense::LGPL_V2_1:
        licenseShort = QCoreApplication::translate("KAboutLicense", "LGPL v2.1", "@item license (short name)");
        licenseFull = QCoreApplication::translate("KAboutLicense", "GNU Lesser General Public License Version 2.1", "@item license");
        break;
    case KAboutLicense::Custom:
    case KAboutLicense::File:
        licenseShort = licenseFull = QCoreApplication::translate("KAboutLicense", "Custom", "@item license");
        break;
    default:
        licenseShort = licenseFull = QCoreApplication::translate("KAboutLicense", "Not specified", "@item license");
    }

    const QString result =
        (formatName == KAboutLicense::ShortName) ? licenseShort :
        (formatName == KAboutLicense::FullName) ?  licenseFull :
        QString();

    return result;
}

KAboutLicense &KAboutLicense::operator=(const KAboutLicense &other)
{
    d = other.d;
    return *this;
}

KAboutLicense::LicenseKey KAboutLicense::key() const
{
    return d->_licenseKey;
}

KAboutLicense KAboutLicense::byKeyword(const QString &rawKeyword)
{
    // Setup keyword->enum dictionary on first call.
    // Use normalized keywords, by the algorithm below.
    static const QHash<QByteArray, KAboutLicense::LicenseKey> licenseDict {
        { "gpl", KAboutLicense::GPL },
        { "gplv2", KAboutLicense::GPL_V2 },
        { "gplv2+", KAboutLicense::GPL_V2 },
        { "gpl20", KAboutLicense::GPL_V2 },
        { "gpl20+", KAboutLicense::GPL_V2 },
        { "lgpl", KAboutLicense::LGPL },
        { "lgplv2", KAboutLicense::LGPL_V2 },
        { "lgplv2+", KAboutLicense::LGPL_V2 },
        { "lgpl20", KAboutLicense::LGPL_V2 },
        { "lgpl20+", KAboutLicense::LGPL_V2 },
        { "bsd", KAboutLicense::BSDL },
        { "bsd2clause", KAboutLicense::BSDL },
        { "artistic", KAboutLicense::Artistic },
        { "artistic10", KAboutLicense::Artistic },
        { "qpl", KAboutLicense::QPL },
        { "qplv1", KAboutLicense::QPL_V1_0 },
        { "qplv10", KAboutLicense::QPL_V1_0 },
        { "qpl10", KAboutLicense::QPL_V1_0 },
        { "gplv3", KAboutLicense::GPL_V3 },
        { "gplv3+", KAboutLicense::GPL_V3 },
        { "gpl30", KAboutLicense::GPL_V3 },
        { "gpl30+", KAboutLicense::GPL_V3 },
        { "lgplv3", KAboutLicense::LGPL_V3 },
        { "lgplv3+", KAboutLicense::LGPL_V3 },
        { "lgpl30", KAboutLicense::LGPL_V3 },
        { "lgpl30+", KAboutLicense::LGPL_V3 },
        { "lgplv21", KAboutLicense::LGPL_V2_1 },
        { "lgplv21+", KAboutLicense::LGPL_V2_1 },
        { "lgpl21", KAboutLicense::LGPL_V2_1 },
        { "lgpl21+", KAboutLicense::LGPL_V2_1 },
    };

    // Normalize keyword.
    QString keyword = rawKeyword;
    keyword = keyword.toLower();
    keyword.remove(QLatin1Char(' '));
    keyword.remove(QLatin1Char('.'));
    keyword.remove(QLatin1Char('-'));

    LicenseKey license = licenseDict.value(keyword.toLatin1(),
                                           KAboutLicense::Custom);
    auto restriction = keyword.endsWith(QLatin1Char('+')) ? OrLaterVersions : OnlyThisVersion;
    return KAboutLicense(license, restriction, nullptr);
}

class Q_DECL_HIDDEN KAboutData::Private
{
public:
    Private()
        : customAuthorTextEnabled(false)
    {}
    QString _componentName;
    QString _displayName;
    QString _shortDescription;
    QString _copyrightStatement;
    QString _otherText;
    QString _homepageAddress;
    QList<KAboutPerson> _authorList;
    QList<KAboutPerson> _creditList;
    QList<KAboutPerson> _translatorList;
    QList<KAboutLicense> _licenseList;
    QString productName;
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 2)
    QString programIconName;
#endif
    QVariant programLogo;
    QString customAuthorPlainText, customAuthorRichText;
    bool customAuthorTextEnabled;

    QString organizationDomain;
    QString _ocsProviderUrl;
    QString desktopFileName;

    // Everything dr.konqi needs, we store as utf-8, so we
    // can just give it a pointer, w/o any allocations.
    QByteArray _internalProgramName;
    QByteArray _version;
    QByteArray _bugAddress;

    static QList<KAboutPerson> parseTranslators(const QString &translatorName, const QString &translatorEmail);

};

KAboutData::KAboutData(const QString &_componentName,
                       const QString &_displayName,
                       const QString &_version,
                       const QString &_shortDescription,
                       enum KAboutLicense::LicenseKey licenseType,
                       const QString &_copyrightStatement,
                       const QString &text,
                       const QString &homePageAddress,
                       const QString &bugAddress
                      )
    : d(new Private)
{
    d->_componentName = _componentName;
    int p = d->_componentName.indexOf(QLatin1Char('/'));
    if (p >= 0) {
        d->_componentName = d->_componentName.mid(p + 1);
    }

    d->_displayName = _displayName;
    if (!d->_displayName.isEmpty()) { // KComponentData("klauncher") gives empty program name
        d->_internalProgramName = _displayName.toUtf8();
    }
    d->_version = _version.toUtf8();
    d->_shortDescription = _shortDescription;
    d->_licenseList.append(KAboutLicense(licenseType, this));
    d->_copyrightStatement = _copyrightStatement;
    d->_otherText = text;
    d->_homepageAddress = homePageAddress;
    d->_bugAddress = bugAddress.toUtf8();

    QUrl homePageUrl(homePageAddress);
    if (!homePageUrl.isValid() || homePageUrl.scheme().isEmpty()) {
        // Default domain if nothing else is better
        homePageUrl.setUrl(QStringLiteral("https://kde.org/"));
    }

    const QChar dotChar(QLatin1Char('.'));
    QStringList hostComponents = homePageUrl.host().split(dotChar);

    // Remove leading component unless 2 (or less) components are present
    if (hostComponents.size() > 2) {
        hostComponents.removeFirst();
    }

    d->organizationDomain = hostComponents.join(dotChar);

    // KF6: do not set a default desktopFileName value here, but remove this code and leave it empty
    // see KAboutData::desktopFileName() for details

    // desktop file name is reverse domain name
    std::reverse(hostComponents.begin(), hostComponents.end());
    hostComponents.append(_componentName);

    d->desktopFileName = hostComponents.join(dotChar);
}

KAboutData::KAboutData(const QString &_componentName,
                       const QString &_displayName,
                       const QString &_version
                      )
    : d(new Private)
{
    d->_componentName = _componentName;
    int p = d->_componentName.indexOf(QLatin1Char('/'));
    if (p >= 0) {
        d->_componentName = d->_componentName.mid(p + 1);
    }

    d->_displayName = _displayName;
    if (!d->_displayName.isEmpty()) { // KComponentData("klauncher") gives empty program name
        d->_internalProgramName = _displayName.toUtf8();
    }
    d->_version = _version.toUtf8();

    // match behaviour of other constructors
    d->_licenseList.append(KAboutLicense(KAboutLicense::Unknown, this));
    d->_bugAddress = "submit@bugs.kde.org";
    d->organizationDomain = QStringLiteral("kde.org");
    // KF6: do not set a default desktopFileName value here, but remove this code and leave it empty
    // see KAboutData::desktopFileName() for details
    d->desktopFileName = QLatin1String("org.kde.") + d->_componentName;
}

KAboutData::~KAboutData()
{
    delete d;
}

KAboutData::KAboutData(const KAboutData &other): d(new Private)
{
    *d = *other.d;
    QList<KAboutLicense>::iterator it = d->_licenseList.begin(), itEnd = d->_licenseList.end();
    for (; it != itEnd; ++it) {
        KAboutLicense &al = *it;
        al.d.detach();
        al.d->_aboutData = this;
    }
}

KAboutData &KAboutData::operator=(const KAboutData &other)
{
    if (this != &other) {
        *d = *other.d;
        QList<KAboutLicense>::iterator it = d->_licenseList.begin(), itEnd = d->_licenseList.end();
        for (; it != itEnd; ++it) {
            KAboutLicense &al = *it;
            al.d.detach();
            al.d->_aboutData = this;
        }
    }
    return *this;
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 65)
KAboutData KAboutData::fromPluginMetaData(const KPluginMetaData &plugin)
{
    KAboutData ret(plugin.pluginId(), plugin.name(), plugin.version(), plugin.description(),
                   KAboutLicense::byKeyword(plugin.license()).key(), plugin.copyrightText(),
                   plugin.extraInformation(), plugin.website());
#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 2)
    ret.d->programIconName = plugin.iconName();
#endif
    ret.d->_authorList = plugin.authors();
    ret.d->_translatorList = plugin.translators();
    ret.d->_creditList = plugin.otherContributors();
    return ret;
}
#endif


KAboutData &KAboutData::addAuthor(const QString &name,
                                  const QString &task,
                                  const QString &emailAddress,
                                  const QString &webAddress,
                                  const QString &ocsUsername)
{
    d->_authorList.append(KAboutPerson(name, task, emailAddress, webAddress, ocsUsername));
    return *this;
}

KAboutData &KAboutData::addCredit(const QString &name,
                                  const QString &task,
                                  const QString &emailAddress,
                                  const QString &webAddress,
                                  const QString &ocsUsername)
{
    d->_creditList.append(KAboutPerson(name, task, emailAddress, webAddress, ocsUsername));
    return *this;
}

KAboutData &KAboutData::setTranslator(const QString &name,
                                      const QString &emailAddress)
{
    d->_translatorList = Private::parseTranslators(name, emailAddress);
    return *this;
}

KAboutData &KAboutData::setLicenseText(const QString &licenseText)
{
    d->_licenseList[0] = KAboutLicense(this);
    d->_licenseList[0].setLicenseFromText(licenseText);
    return *this;
}

KAboutData &KAboutData::addLicenseText(const QString &licenseText)
{
    // if the default license is unknown, overwrite instead of append
    KAboutLicense &firstLicense = d->_licenseList[0];
    KAboutLicense newLicense(this);
    newLicense.setLicenseFromText(licenseText);
    if (d->_licenseList.count() == 1 && firstLicense.d->_licenseKey == KAboutLicense::Unknown) {
        firstLicense = newLicense;
    } else {
        d->_licenseList.append(newLicense);
    }

    return *this;
}

KAboutData &KAboutData::setLicenseTextFile(const QString &pathToFile)
{
    d->_licenseList[0] = KAboutLicense(this);
    d->_licenseList[0].setLicenseFromPath(pathToFile);
    return *this;
}

KAboutData &KAboutData::addLicenseTextFile(const QString &pathToFile)
{
    // if the default license is unknown, overwrite instead of append
    KAboutLicense &firstLicense = d->_licenseList[0];
    KAboutLicense newLicense(this);
    newLicense.setLicenseFromPath(pathToFile);
    if (d->_licenseList.count() == 1 && firstLicense.d->_licenseKey == KAboutLicense::Unknown) {
        firstLicense = newLicense;
    } else {
        d->_licenseList.append(newLicense);
    }
    return *this;
}

KAboutData &KAboutData::setComponentName(const QString &componentName)
{
    d->_componentName = componentName;
    return *this;
}

KAboutData &KAboutData::setDisplayName(const QString &_displayName)
{
    d->_displayName = _displayName;
    d->_internalProgramName = _displayName.toUtf8();
    return *this;
}

KAboutData &KAboutData::setOcsProvider(const QString &_ocsProviderUrl)
{
    d->_ocsProviderUrl = _ocsProviderUrl;
    return *this;
}

KAboutData &KAboutData::setVersion(const QByteArray &_version)
{
    d->_version = _version;
    return *this;
}

KAboutData &KAboutData::setShortDescription(const QString &_shortDescription)
{
    d->_shortDescription = _shortDescription;
    return *this;
}

KAboutData &KAboutData::setLicense(KAboutLicense::LicenseKey licenseKey)
{
    return setLicense(licenseKey, KAboutLicense::OnlyThisVersion);
}

KAboutData &KAboutData::setLicense(KAboutLicense::LicenseKey licenseKey,
                                   KAboutLicense::VersionRestriction versionRestriction)
{
    d->_licenseList[0] = KAboutLicense(licenseKey, versionRestriction, this);
    return *this;
}

KAboutData &KAboutData::addLicense(KAboutLicense::LicenseKey licenseKey)
{
    return addLicense(licenseKey, KAboutLicense::OnlyThisVersion);
}

KAboutData &KAboutData::addLicense(KAboutLicense::LicenseKey licenseKey,
                                   KAboutLicense::VersionRestriction versionRestriction)
{
    // if the default license is unknown, overwrite instead of append
    KAboutLicense &firstLicense = d->_licenseList[0];
    if (d->_licenseList.count() == 1 && firstLicense.d->_licenseKey == KAboutLicense::Unknown) {
        firstLicense = KAboutLicense(licenseKey, versionRestriction, this);
    } else {
        d->_licenseList.append(KAboutLicense(licenseKey, versionRestriction, this));
    }
    return *this;
}

KAboutData &KAboutData::setCopyrightStatement(const QString &_copyrightStatement)
{
    d->_copyrightStatement = _copyrightStatement;
    return *this;
}

KAboutData &KAboutData::setOtherText(const QString &_otherText)
{
    d->_otherText = _otherText;
    return *this;
}

KAboutData &KAboutData::setHomepage(const QString &homepage)
{
    d->_homepageAddress = homepage;
    return *this;
}

KAboutData &KAboutData::setBugAddress(const QByteArray &_bugAddress)
{
    d->_bugAddress = _bugAddress;
    return *this;
}

KAboutData &KAboutData::setOrganizationDomain(const QByteArray &domain)
{
    d->organizationDomain = QString::fromLatin1(domain.data());
    return *this;
}

KAboutData &KAboutData::setProductName(const QByteArray &_productName)
{
    d->productName = QString::fromUtf8(_productName.data());
    return *this;
}

QString KAboutData::componentName() const
{
    return d->_componentName;
}

QString KAboutData::productName() const
{
    if (!d->productName.isEmpty()) {
        return d->productName;
    }
    return componentName();
}

QString KAboutData::displayName() const
{
    if (!d->_displayName.isEmpty()) {
        return d->_displayName;
    }
    return componentName();
}

/// @internal
/// Return the program name. It is always pre-allocated.
/// Needed for KCrash in particular.
const char *KAboutData::internalProgramName() const
{
    return d->_internalProgramName.constData();
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(5, 2)
QString KAboutData::programIconName() const
{
    return d->programIconName.isEmpty() ? componentName() : d->programIconName;
}

KAboutData &KAboutData::setProgramIconName(const QString &iconName)
{
    d->programIconName = iconName;
    return *this;
}
#endif

QVariant KAboutData::programLogo() const
{
    return d->programLogo;
}

KAboutData &KAboutData::setProgramLogo(const QVariant &image)
{
    d->programLogo = image;
    return *this;
}

QString KAboutData::ocsProviderUrl() const
{
    return d->_ocsProviderUrl;
}

QString KAboutData::version() const
{
    return QString::fromUtf8(d->_version.data());
}

/// @internal
/// Return the untranslated and uninterpreted (to UTF8) string
/// for the version information. Used in particular for KCrash.
const char *KAboutData::internalVersion() const
{
    return d->_version.constData();
}

QString KAboutData::shortDescription() const
{
    return d->_shortDescription;
}

QString KAboutData::homepage() const
{
    return d->_homepageAddress;
}

QString KAboutData::bugAddress() const
{
    return QString::fromUtf8(d->_bugAddress.constData());
}

QString KAboutData::organizationDomain() const
{
    return d->organizationDomain;
}

/// @internal
/// Return the untranslated and uninterpreted (to UTF8) string
/// for the bug mail address. Used in particular for KCrash.
const char *KAboutData::internalBugAddress() const
{
    if (d->_bugAddress.isEmpty()) {
        return nullptr;
    }
    return d->_bugAddress.constData();
}

QList<KAboutPerson> KAboutData::authors() const
{
    return d->_authorList;
}

QList<KAboutPerson> KAboutData::credits() const
{
    return d->_creditList;
}

QList<KAboutPerson> KAboutData::Private::parseTranslators(const QString &translatorName, const QString &translatorEmail)
{
    QList<KAboutPerson> personList;
    if (translatorName.isEmpty() || translatorName == QLatin1String("Your names")) {
        return personList;
    }

    const QStringList nameList(translatorName.split(QLatin1Char(',')));

    QStringList emailList;
    if (!translatorEmail.isEmpty() && translatorEmail != QLatin1String("Your emails")) {
        emailList = translatorEmail.split(QLatin1Char(','), QString::KeepEmptyParts);
    }

    QStringList::const_iterator nit;
    QStringList::const_iterator eit = emailList.constBegin();

    for (nit = nameList.constBegin(); nit != nameList.constEnd(); ++nit) {
        QString email;
        if (eit != emailList.constEnd()) {
            email = *eit;
            ++eit;
        }

        personList.append(KAboutPerson((*nit).trimmed(), email.trimmed(), true));
    }

    return personList;
}

QList<KAboutPerson> KAboutData::translators() const
{
    return d->_translatorList;
}


QString KAboutData::aboutTranslationTeam()
{
    return QCoreApplication::translate(
               "KAboutData",
               "<p>KDE is translated into many languages thanks to the work "
               "of the translation teams all over the world.</p>"
               "<p>For more information on KDE internationalization "
               "visit <a href=\"https://l10n.kde.org\">https://l10n.kde.org</a></p>",
               "replace this with information about your translation team"
           );
}

QString KAboutData::otherText() const
{
    return d->_otherText;
}

QList<KAboutLicense> KAboutData::licenses() const
{
    return d->_licenseList;
}

QString KAboutData::copyrightStatement() const
{
    return d->_copyrightStatement;
}

QString KAboutData::customAuthorPlainText() const
{
    return d->customAuthorPlainText;
}

QString KAboutData::customAuthorRichText() const
{
    return d->customAuthorRichText;
}

bool KAboutData::customAuthorTextEnabled() const
{
    return d->customAuthorTextEnabled;
}

KAboutData &KAboutData::setCustomAuthorText(const QString &plainText,
        const QString &richText)
{
    d->customAuthorPlainText = plainText;
    d->customAuthorRichText = richText;

    d->customAuthorTextEnabled = true;

    return *this;
}

KAboutData &KAboutData::unsetCustomAuthorText()
{
    d->customAuthorPlainText = QString();
    d->customAuthorRichText = QString();

    d->customAuthorTextEnabled = false;

    return *this;
}

KAboutData &KAboutData::setDesktopFileName(const QString &desktopFileName)
{
    d->desktopFileName = desktopFileName;

    return *this;
}

QString KAboutData::desktopFileName() const
{
    return d->desktopFileName;
    // KF6: switch to this code and adapt API dox
#if 0
    // if desktopFileName has been explicitly set, use that value
    if (!d->desktopFileName.isEmpty()) {
        return d->desktopFileName;
    }

    // return a string calculated on-the-fly from the current org domain & component name
    const QChar dotChar(QLatin1Char('.'));
    QStringList hostComponents = d->organizationDomain.split(dotChar);

    // desktop file name is reverse domain name
    std::reverse(hostComponents.begin(), hostComponents.end());
    hostComponents.append(componentName());

    return hostComponents.join(dotChar);
#endif
}

class KAboutDataRegistry
{
public:
    KAboutDataRegistry() : m_appData(nullptr) {}
    ~KAboutDataRegistry()
    {
        delete m_appData;
        qDeleteAll(m_pluginData);
    }
    KAboutDataRegistry(const KAboutDataRegistry &) = delete;
    KAboutDataRegistry &operator=(const KAboutDataRegistry &) = delete;

    KAboutData *m_appData;
    QHash<QString, KAboutData *> m_pluginData;
};

Q_GLOBAL_STATIC(KAboutDataRegistry, s_registry)

namespace {

void warnIfOutOfSync(const char *aboutDataString, const QString &aboutDataValue,
                     const char *appDataString, const QString &appDataValue)
{
    if (aboutDataValue != appDataValue) {
        qCWarning(KABOUTDATA) << appDataString <<appDataValue << "is out-of-sync with" << aboutDataString << aboutDataValue;
    }
}

}

KAboutData KAboutData::applicationData()
{
    QCoreApplication *app = QCoreApplication::instance();

    KAboutData *aboutData = s_registry->m_appData;

    // not yet existing
    if (!aboutData) {
        // init from current Q*Application data
        aboutData = new KAboutData(QCoreApplication::applicationName(),
                                   QString(),
                                   QString());
        // For applicationDisplayName & desktopFileName, which are only properties of QGuiApplication,
        // we have to try to get them via the property system, as the static getter methods are
        // part of QtGui only. Disadvantage: requires an app instance.
        // Either get all or none of the properties & warn about it
        if (app) {
            aboutData->setOrganizationDomain(QCoreApplication::organizationDomain().toUtf8());
            aboutData->setVersion(QCoreApplication::applicationVersion().toUtf8());
            aboutData->setDisplayName(app->property("applicationDisplayName").toString());
            aboutData->setDesktopFileName(app->property("desktopFileName").toString());
        } else {
            qCWarning(KABOUTDATA) << "Could not initialize the properties of KAboutData::applicationData by the equivalent properties from Q*Application: no app instance (yet) existing.";
        }

        s_registry->m_appData = aboutData;
    } else {
        // check if in-sync with Q*Application metadata, as their setters could have been called
        // after the last KAboutData::setApplicationData, with different values
        warnIfOutOfSync("KAboutData::applicationData().componentName", aboutData->componentName(),
                        "QCoreApplication::applicationName", QCoreApplication::applicationName());
        warnIfOutOfSync("KAboutData::applicationData().version", aboutData->version(),
                        "QCoreApplication::applicationVersion", QCoreApplication::applicationVersion());
        warnIfOutOfSync("KAboutData::applicationData().organizationDomain", aboutData->organizationDomain(),
                        "QCoreApplication::organizationDomain", QCoreApplication::organizationDomain());
        if (app) {
            warnIfOutOfSync("KAboutData::applicationData().displayName", aboutData->displayName(),
                            "QGuiApplication::applicationDisplayName", app->property("applicationDisplayName").toString());
            warnIfOutOfSync("KAboutData::applicationData().desktopFileName", aboutData->desktopFileName(),
                            "QGuiApplication::desktopFileName", app->property("desktopFileName").toString());
        }
    }

    return *aboutData;
}

void KAboutData::setApplicationData(const KAboutData &aboutData)
{
    if (s_registry->m_appData) {
        *s_registry->m_appData = aboutData;
    } else {
        s_registry->m_appData = new KAboutData(aboutData);
    }

    // For applicationDisplayName & desktopFileName, which are only properties of QGuiApplication,
    // we have to try to set them via the property system, as the static getter methods are
    // part of QtGui only. Disadvantage: requires an app instance.
    // So set either all or none of the properties & warn about it
    QCoreApplication *app = QCoreApplication::instance();
    if (app) {
        app->setApplicationVersion(aboutData.version());
        app->setApplicationName(aboutData.componentName());
        app->setOrganizationDomain(aboutData.organizationDomain());
        app->setProperty("applicationDisplayName", aboutData.displayName());
        app->setProperty("desktopFileName", aboutData.desktopFileName());
    } else {
        qCWarning(KABOUTDATA) << "Could not initialize the equivalent properties of Q*Application: no instance (yet) existing.";
    }

    // KF6: Rethink the current relation between KAboutData::applicationData and the Q*Application metadata
    // Always overwriting the Q*Application metadata here, but not updating back the KAboutData
    // in applicationData() is unbalanced and can result in out-of-sync data if the Q*Application
    // setters have been called meanwhile
    // Options are to remove the overlapping properties of KAboutData for cleancode, or making the
    // overlapping properties official shadow properties of their Q*Application countparts, though
    // that increases behavioural complexity a little.
}

void KAboutData::registerPluginData(const KAboutData &aboutData)
{
    s_registry->m_pluginData.insert(aboutData.componentName(), new KAboutData(aboutData));
}

KAboutData *KAboutData::pluginData(const QString &componentName)
{
    KAboutData *ad = s_registry->m_pluginData.value(componentName);
    return ad;
}

// only for KCrash (no memory allocation allowed)
const KAboutData *KAboutData::applicationDataPointer()
{
    if (s_registry.exists()) {
        return s_registry->m_appData;
    }
    return nullptr;
}

bool KAboutData::setupCommandLine(QCommandLineParser *parser)
{
    if (!d->_shortDescription.isEmpty()) {
        parser->setApplicationDescription(d->_shortDescription);
    }

    parser->addHelpOption();

    QCoreApplication *app = QCoreApplication::instance();
    if (app && !app->applicationVersion().isEmpty()) {
        parser->addVersionOption();
    }

    return parser->addOption(QCommandLineOption(QStringLiteral("author"), QCoreApplication::translate("KAboutData CLI", "Show author information.")))
           && parser->addOption(QCommandLineOption(QStringLiteral("license"), QCoreApplication::translate("KAboutData CLI", "Show license information.")))
           && parser->addOption(QCommandLineOption(QStringLiteral("desktopfile"),
                                                   QCoreApplication::translate("KAboutData CLI", "The base file name of the desktop entry for this application."),
                                                   QCoreApplication::translate("KAboutData CLI", "file name")));
}

void KAboutData::processCommandLine(QCommandLineParser *parser)
{
    bool foundArgument = false;
    if (parser->isSet(QStringLiteral("author"))) {
        foundArgument = true;
        if (d->_authorList.isEmpty()) {
            printf("%s\n", qPrintable(QCoreApplication::translate("KAboutData CLI", "This application was written by somebody who wants to remain anonymous.")));
        } else {
            printf("%s\n", qPrintable(QCoreApplication::translate("KAboutData CLI", "%1 was written by:").arg(qAppName())));
            for (const KAboutPerson &person : qAsConst(d->_authorList)) {
                QString authorData = QLatin1String("    ") + person.name();
                if (!person.emailAddress().isEmpty()) {
                    authorData.append(QLatin1String(" <") + person.emailAddress() + QLatin1Char('>'));
                }
                printf("%s\n", qPrintable(authorData));
            }
        }
        if (!customAuthorTextEnabled()) {
            if (bugAddress() == QLatin1String("submit@bugs.kde.org") ) {
                printf("%s\n", qPrintable(QCoreApplication::translate("KAboutData CLI", "Please use https://bugs.kde.org to report bugs.")));
            } else if (!bugAddress().isEmpty()) {
                printf("%s\n", qPrintable(QCoreApplication::translate("KAboutData CLI", "Please report bugs to %1.").arg(bugAddress())));
            }
        } else {
            printf("%s\n", qPrintable(customAuthorPlainText()));
        }
    } else if (parser->isSet(QStringLiteral("license"))) {
        foundArgument = true;
        for (const KAboutLicense &license : qAsConst(d->_licenseList)) {
            printf("%s\n", qPrintable(license.text()));
        }
    }

    const QString desktopFileName = parser->value(QStringLiteral("desktopfile"));
    if (!desktopFileName.isEmpty()) {
        d->desktopFileName = desktopFileName;
    }

    if (foundArgument) {
        ::exit(EXIT_SUCCESS);
    }
}

template <class T>
QVariantList listToVariant(const QList<T>& values)
{
    QVariantList ret;
    ret.reserve(values.count());
    for(const auto &license: values) {
        ret << QVariant::fromValue(license);
    }
    return ret;
}

QVariantList KAboutData::licensesVariant() const
{
    return listToVariant(d->_licenseList);
}

QVariantList KAboutData::authorsVariant() const
{
    return listToVariant(d->_authorList);
}

QVariantList KAboutData::creditsVariant() const
{
    return listToVariant(d->_creditList);
}

QVariantList KAboutData::translatorsVariant() const
{
    return listToVariant(d->_translatorList);
}
