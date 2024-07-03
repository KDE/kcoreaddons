/*
    This file is part of the KDE Libraries

    SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>
    SPDX-FileCopyrightText: 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>
    SPDX-FileCopyrightText: 2013 David Faure <faure+bluesystems@kde.org>
    SPDX-FileCopyrightText: 2017 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2021 Julius Künzel <jk.kdedev@smartlab.uber.space>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KABOUTDATA_H
#define KABOUTDATA_H

#include <QSharedDataPointer>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <kcoreaddons_export.h>
#include <memory>
#include <qcontainerfwd.h>

class QCommandLineParser;
class QJsonObject;
class KAboutData;
namespace KCrash
{
#ifdef KCOREADDONS_STATIC
void defaultCrashHandler(int sig);
#else
Q_DECL_IMPORT void defaultCrashHandler(int sig);
#endif
}

/*!
 * \class KAboutPerson
 * \inheaderfile KAboutData
 * \inmodule KCoreAddons
 *
 * This class is used to store information about a person or developer.
 * It can store the person's name, a task, an email address and a
 * link to a home page. This class is intended for use in the
 * KAboutData class, but it can be used elsewhere as well.
 * Normally you should at least define the person's name.
 * Creating a KAboutPerson object by yourself is relatively useless,
 * but the KAboutData methods KAboutData::authors() and KAboutData::credits()
 * return lists of KAboutPerson data objects which you can examine.
 *
 * Example usage within a main(), retrieving the list of people involved
 * with a program and re-using data from one of them:
 *
 * \code
 * KAboutData about("khello", i18n("KHello"), "0.1",
 *                   i18n("A KDE version of Hello, world!"),
 *                   KAboutLicense::LGPL,
 *                   i18n("Copyright (C) 2014 Developer"));
 *
 * about.addAuthor(i18n("Joe Developer"), i18n("developer"), "joe@host.com", 0);
 * QList<KAboutPerson> people = about.authors();
 * about.addCredit(people[0].name(), people[0].task());
 * \endcode
 */
class KCOREADDONS_EXPORT KAboutPerson
{
    Q_GADGET
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString task READ task CONSTANT)
    Q_PROPERTY(QString emailAddress READ emailAddress CONSTANT)
    Q_PROPERTY(QString webAddress READ webAddress CONSTANT)
    Q_PROPERTY(QUrl avatarUrl READ avatarUrl CONSTANT)
    friend class KAboutData;
    friend class KAboutDataPrivate;

public:
    /*!
     * Convenience constructor
     *
     * \a name The name of the person.
     *
     * \a task The task of this person.
     *
     * \a emailAddress The email address of the person.
     *
     * \a webAddress Home page of the person.
     *
     * \a avatarUrl URL to the avatar of the person, since 6.0
     *
     * \a name default argument, since 5.53
     */
    explicit KAboutPerson(const QString &name = QString(),
                          const QString &task = QString(),
                          const QString &emailAddress = QString(),
                          const QString &webAddress = QString(),
                          const QUrl &avatarUrl = QUrl());

    /*!
     * Copy constructor.  Performs a deep copy.
     *
     * \a other object to copy
     */
    KAboutPerson(const KAboutPerson &other);

    ~KAboutPerson();

    /*!
     * Assignment operator.  Performs a deep copy.
     *
     * \a other object to copy
     */
    KAboutPerson &operator=(const KAboutPerson &other);

    /*!
     * The person's name
     * Returns the person's name (can be QString(), if it has been
     *           constructed with an empty name)
     */
    QString name() const;

    /*!
     * The person's task
     * Returns the person's task (can be QString(), if it has been
     *           constructed with an empty task)
     */
    QString task() const;

    /*!
     * The person's email address
     * Returns the person's email address (can be QString(), if it has been
     *           constructed with an empty email)
     */
    QString emailAddress() const;

    /*!
     * The home page or a relevant link
     * Returns the persons home page (can be QString(), if it has been
     *           constructed with an empty home page)
     */
    QString webAddress() const;

    /*!
     * Returns an URL pointing to the user's avatar
     * \since KCoreAddons 6.0
     */
    QUrl avatarUrl() const;

    /*!
      Creates a \c KAboutPerson from a JSON object with the following structure:

     \table
        \header
            \li Key
            \li Accessor
        \row
            \li Name
            \li name()
        \row
            \li EMail
            \li emailAddress()
        \row
            \li Task
            \li task()
        \row
            \li Website
            \li webAddress()
        \row
            \li AvatarUrl
            \li avatarUrl()
        \endtable

      The \c Name and \c Task key are translatable (by using e.g. a "Task[de_DE]" key)
      The AvatarUrl exists since version 6.0

      \since KCoreAddons 5.18
     */
    static KAboutPerson fromJSON(const QJsonObject &obj);

private:
    /*!
     * \internal Used by KAboutData to construct translator data.
     */
    KCOREADDONS_NO_EXPORT explicit KAboutPerson(const QString &name, const QString &email, bool disambiguation);

private:
    QSharedDataPointer<class KAboutPersonPrivate> d;
};

Q_DECLARE_TYPEINFO(KAboutPerson, Q_RELOCATABLE_TYPE);

/*!
 * \class KAboutLicense
 * \inmodule KCoreAddons
 * \inheaderfile KAboutData
 *
 * This class is used to store information about a license.
 * The license can be one of some predefined, one given as text or one
 * that can be loaded from a file. This class is used in the KAboutData class.
 * Explicitly creating a KAboutLicense object is not possible.
 * If the license is wanted for a KDE component having KAboutData object,
 * use KAboutData::licenses() to get the licenses for that component.
 * If the license is for a non-code resource and given by a keyword
 * (e.g. in .desktop files), try using KAboutLicense::byKeyword().
 */
class KCOREADDONS_EXPORT KAboutLicense
{
    Q_GADGET
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString text READ text CONSTANT)
    Q_PROPERTY(KAboutLicense::LicenseKey key READ key CONSTANT)
    Q_PROPERTY(QString spdx READ spdx CONSTANT)
    friend class KAboutData;
    friend class KAboutComponent;

public:
    /*!
      Describes the license of the software; for more information see: https://spdx.org/licenses/

      \value Custom Custom license
      \value File License set from text file, see setLicenseFromPath()
      \value Unknown Unknown license
      \value GPL GPL
      \value GPL_V2 GPL_V2, this has the same value as LicenseKey::GPL, see https://spdx.org/licenses/GPL-2.0.html
      \value LGPL LGPL
      \value LGPL_V2 LGPL_V2, this has the same value as LicenseKey::LGPL, see https://spdx.org/licenses/LGPL-2.0-only.html
      \value BSDL BSDL, see https://spdx.org/licenses/BSD-2-Clause.html
      \value Artistic Artistic, see https://spdx.org/licenses/Artistic-2.0.html
      \value GPL_V3 GPL_V3, see https://spdx.org/licenses/GPL-3.0.html
      \value LGPL_V3 LGPL_V3, see https://spdx.org/licenses/LGPL-3.0-only.html
      \value [since 5.25] LGPL_V2_1 LGPL_V2_1, see https://spdx.org/licenses/LGPL-2.1-only.html
      \value [since 6.0] MIT, see https://spdx.org/licenses/MIT.html
     */
    enum LicenseKey {
        Custom = -2, ///< Custom license
        File = -1, ///< License set from text file, see setLicenseFromPath()
        Unknown = 0, ///< Unknown license
        GPL = 1, ///< GPL
        GPL_V2 = GPL, ///< GPL_V2, this has the same value as LicenseKey::GPL, see https://spdx.org/licenses/GPL-2.0.html
        LGPL = 2, ///< LGPL
        LGPL_V2 = LGPL, ///< LGPL_V2, this has the same value as LicenseKey::LGPL, see https://spdx.org/licenses/LGPL-2.0-only.html
        BSDL = 3, ///< BSDL, see https://spdx.org/licenses/BSD-2-Clause.html
        Artistic = 4, ///< Artistic, see https://spdx.org/licenses/Artistic-2.0.html
        GPL_V3 = 5, ///< GPL_V3, see https://spdx.org/licenses/GPL-3.0.html
        LGPL_V3 = 6, ///< LGPL_V3, see https://spdx.org/licenses/LGPL-3.0-only.html
        LGPL_V2_1 = 7, ///< LGPL_V2_1 \since KCoreAddons 5.25, see https://spdx.org/licenses/LGPL-2.1-only.html
        MIT = 8, ///< MIT \since KCoreAddons 6.0, see https://spdx.org/licenses/MIT.html
    };
    Q_ENUM(LicenseKey)

    /*!
     * \since KCoreAddons 6.0
     * Format of the license name.
     *
     * \value ShortFormat Short format
     * \value FullName Full name
     *
     */
    enum NameFormat {
        ShortName,
        FullName,
    };
    Q_ENUM(NameFormat)

    /*!
     * \since KCoreAddons 6.0
     * Whether later versions of the license are allowed.
     *
     * \value OnlyThisVersion Only this version of the license is allowed
     * \valie OrLaterVersions Any later version of the license is allowed
     *
     */
    enum VersionRestriction {
        OnlyThisVersion,
        OrLaterVersions,
    };
    Q_ENUM(VersionRestriction)

    /*!
     * \since KCoreAddons 5.53
     */
    explicit KAboutLicense();

    /*!
     * Copy constructor.  Performs a deep copy.
     *
     * \a other object to copy
     */
    KAboutLicense(const KAboutLicense &other);

    ~KAboutLicense();

    /*!
     * Assignment operator.  Performs a deep copy.
     *
     * \a other object to copy
     */
    KAboutLicense &operator=(const KAboutLicense &other);

    /*!
     * Returns the full license text. If the licenseType argument of the
     * constructor has been used, any text defined by setLicenseText is ignored,
     * and the standard text for the chosen license will be returned.
     *
     * Returns the license text.
     */
    QString text() const;

    /*!
     * Returns the license name.
     *
     * Default argument \since KCoreAddons 5.53
     *
     * Returns The license name as a string.
     */
    QString name(KAboutLicense::NameFormat formatName = ShortName) const;

    /*!
     * Returns the license key.
     *
     * Returns The license key as element of KAboutLicense::LicenseKey enum.
     */
    KAboutLicense::LicenseKey key() const;

    /*!
     * Returns the SPDX license expression of this license.
     * If the underlying license cannot be expressed as a SPDX expression a null string is returned.
     *
     * \note SPDX expression are expansive constructs. If you parse the return value, do it in a
     *   SPDX specification compliant manner by splitting on whitespaces to discard unwanted
     *   information or by using a complete SPDX license expression parser.
     * \note SPDX identifiers are case-insensitive. Do not use case-sensitive checks on the return
     *   value.
     *
     * See \l https://spdx.org/licenses
     * Returns SPDX license expression or QString() if the license has no identifier. Compliant
     *   with SPDX 2.1.
     *
     * \since KCoreAddons 5.37
     */
    QString spdx() const;

    /*!
     * Fetch a known license by a keyword/spdx ID
     *
     * Frequently the license data is provided by a terse keyword-like string,
     * e.g. by a field in a .desktop file. Using this method, an application
     * can get hold of a proper KAboutLicense object, providing that the
     * license is one of the several known to KDE, and use it to present
     * more human-readable information to the user.
     *
     * Keywords are matched by stripping all whitespace and lowercasing.
     * The known keywords correspond to the KAboutLicense::LicenseKey enumeration,
     * e.g. any of "LGPLV3", "LGPLv3", "LGPL v3" would match KAboutLicense::LGPL_V3.
     * If there is no match for the keyword, a valid license object is still
     * returned, with its name and text informing about a custom license,
     * and its key equal to KAboutLicense::Custom.
     *
     * \a keyword The license keyword.
     *
     * Returns The license object.
     *
     * \sa KAboutLicense::LicenseKey
     */
    static KAboutLicense byKeyword(const QString &keyword);

private:
    /*!
     * \internal Used by KAboutData to construct a predefined license.
     */
    KCOREADDONS_NO_EXPORT explicit KAboutLicense(KAboutLicense::LicenseKey licenseType,
                                                 KAboutLicense::VersionRestriction versionRestriction,
                                                 const KAboutData *aboutData);
    /*!
     * \internal Used by KAboutData to construct a predefined license.
     */
    KCOREADDONS_NO_EXPORT explicit KAboutLicense(enum KAboutLicense::LicenseKey licenseType, const KAboutData *aboutData);
    /*!
     * \internal Used by KAboutData to construct a KAboutLicense
     */
    KCOREADDONS_NO_EXPORT explicit KAboutLicense(const KAboutData *aboutData);
    /*!
     * \internal Used by KAboutData to construct license by given text
     */
    KCOREADDONS_NO_EXPORT void setLicenseFromPath(const QString &pathToFile);
    /*!
     * \internal Used by KAboutData to construct license by given text
     */
    KCOREADDONS_NO_EXPORT void setLicenseFromText(const QString &licenseText);

private:
    QSharedDataPointer<class KAboutLicensePrivate> d;
};

Q_DECLARE_TYPEINFO(KAboutLicense, Q_RELOCATABLE_TYPE);

/*!
 * \class KAboutComponent
 * \inheaderfile KAboutData
 * \inmodule KCoreAddons
 *
 * This class is used to store information about a third party component.
 * It can store the component's name, a description, a link to a website
 * and the license of the libary. This class is intended for use in the
 * KAboutData class, but it can be used elsewhere as well.
 * Normally you should at least define the libary's name.
 * Creating a KAboutComponent object by yourself is relatively useless,
 * but the KAboutData method KAboutData::libaries() return lists of
 * KAboutComponent data objects which you can examine.
 *
 * Example usage within a main(), retrieving the list of components used
 * by a program and re-using data from one of them:
 *
 * \code
 * KAboutData about("khello", i18n("KHello"), "0.1",
 *                   i18n("A KDE version of Hello, world!"),
 *                   KAboutLicense::LGPL,
 *                   i18n("Copyright (C) 2014 Developer"));
 *
 * about.addComponent(i18n("Awsom Lib"),
 *                  i18n("Does awesom stuff. Copyright (C) 2014"),
 *                  i18n("1.02.3"),
 *                  "http://example.com",
 *                  KAboutLicense::LGPL);
 * QList<KAboutComponent> components = about.components();
 * \endcode
 *
 * \since KCoreAddons 5.84
 */
class KCOREADDONS_EXPORT KAboutComponent
{
    Q_GADGET
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString webAddress READ webAddress CONSTANT)
    Q_PROPERTY(KAboutLicense licenses READ license CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    friend class KAboutData;
    friend class KAboutDataPrivate;

public:
    /*!
     * Convenience constructor
     *
     * \a name The name of the component.
     *
     * \a description The description of this component.
     *
     * \a version The version of this component.
     *
     * \a webAddress Website of the component.
     *
     * \a licenseType The license identifier of the component.
     *
     * \a name default argument
     */
    explicit KAboutComponent(const QString &name = QString(),
                             const QString &description = QString(),
                             const QString &version = QString(),
                             const QString &webAddress = QString(),
                             enum KAboutLicense::LicenseKey licenseType = KAboutLicense::Unknown);

    /*!
     * Convenience constructor
     *
     * \a name The name of the component.
     *
     * \a description The description of this component.
     *
     * \a version The version of this component.
     *
     * \a webAddress Website of the component.
     *
     * \a pathToLicenseFile Path to the file in the local filesystem containing the license text.
     *        The file format has to be plain text in an encoding compatible to the local.
     *
     * \a name default argument
     */
    explicit KAboutComponent(const QString &name,
                             const QString &description,
                             const QString &version,
                             const QString &webAddress,
                             const QString &pathToLicenseFile);

    /*!
     * Copy constructor. Performs a deep copy.
     *
     * \a other object to copy
     */
    KAboutComponent(const KAboutComponent &other);

    ~KAboutComponent();

    /*!
     * Assignment operator. Performs a deep copy.
     *
     * \a other object to copy
     */
    KAboutComponent &operator=(const KAboutComponent &other);

    /*!
     * The component's name
     * Returns the component's name (can be QString(), if it has been
     *           constructed with an empty name)
     */
    QString name() const;

    /*!
     * The component's description
     * Returns the component's description (can be empty)
     */
    QString description() const;

    /*!
     * The component's version
     * Returns the component's task (can be empty)
     */
    QString version() const;

    /*!
     * The website or a relevant link
     * Returns the component's website (can be empty)
     */
    QString webAddress() const;

    /*!
     * The component's license
     * Returns the component's KAboutLicense
     */
    KAboutLicense license() const;

private:
    QSharedDataPointer<class KAboutComponentPrivate> d;
};

Q_DECLARE_TYPEINFO(KAboutComponent, Q_RELOCATABLE_TYPE);

/*!
 * \class KAboutData
 *
 * This class is used to store information about a program or plugin.
 * It can store such values as version number, program name, home page, address
 * for bug reporting, multiple authors and contributors
 * (using KAboutPerson), license and copyright information.
 *
 * Currently, the values set here are shown by the "About" box
 * (see KAboutApplicationDialog), used by the bug report dialog (see KBugReport),
 * and by the help shown on command line (see KAboutData::setupCommandLine()).
 *
 * Porting Notes: Since KDE Frameworks 5.0, the translation catalog mechanism
 * must be provided by your translation framework to load the correct catalog
 * instead (eg: KLocalizedString::setApplicationDomain() for KI18n, or
 * QCoreApplication::installTranslator() for Qt's translation system). This
 * applies to the old setCatalogName() and catalogName() members. But see also
 * K4AboutData in kde4support as a compatibility class.
 *
 * Example:
 * Setting the metadata of an application using KAboutData in code also relying
 * on the KDE Framework modules KI18n and KDBusAddons:
 * \code
 * // create QApplication instance
 * QApplication app(argc, argv);
 * // setup translation string domain for the i18n calls
 * KLocalizedString::setApplicationDomain("foo");
 * // create a KAboutData object to use for setting the application metadata
 * KAboutData aboutData("foo", i18n("Foo"), "0.1",
 *                      i18n("To Foo or not To Foo"),
 *                      KAboutLicense::LGPL,
 *                      i18n("Copyright 2017 Bar Foundation"), QString(),
 *                      "https://www.foo-the-app.net");
 * // overwrite default-generated values of organizationDomain & desktopFileName
 * aboutData.setOrganizationDomain("barfoundation.org");
 * aboutData.setDesktopFileName("org.barfoundation.foo");
 *
 * // set the application metadata
 * KAboutData::setApplicationData(aboutData);
 * // in GUI apps set the window icon manually, not covered by KAboutData
 * // needed for environments where the icon name is not extracted from
 * // the information in the application's desktop file
 * QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("foo")));
 *
 * // integrate with commandline argument handling
 * QCommandLineParser parser;
 * aboutData.setupCommandLine(&parser);
 * // setup of app specific commandline args
 * [...]
 * parser.process(app);
 * aboutData.processCommandLine(&parser);
 *
 * // with the application metadata set, register to the D-Bus session
 * KDBusService programDBusService(KDBusService::Multiple | KDBusService::NoExitOnFailure);
 * \endcode
 *
 * \brief Holds information needed by the "About" box and other
 * classes.
 *
 */
class KCOREADDONS_EXPORT KAboutData
{
    Q_GADGET
    Q_PROPERTY(QString displayName READ displayName CONSTANT)
    Q_PROPERTY(QString productName READ productName CONSTANT)
    Q_PROPERTY(QString componentName READ componentName CONSTANT)
    Q_PROPERTY(QVariant programLogo READ programLogo CONSTANT)
    Q_PROPERTY(QString shortDescription READ shortDescription CONSTANT)
    Q_PROPERTY(QString homepage READ homepage CONSTANT)
    Q_PROPERTY(QString bugAddress READ bugAddress CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString otherText READ otherText CONSTANT)
    Q_PROPERTY(QList<KAboutPerson> authors READ authors CONSTANT) // constant in practice as addAuthor is not exposed to Q_GADGET
    Q_PROPERTY(QList<KAboutPerson> credits READ credits CONSTANT)
    Q_PROPERTY(QList<KAboutPerson> translators READ translators CONSTANT)
    Q_PROPERTY(QList<KAboutComponent> components READ components CONSTANT)
    Q_PROPERTY(QList<KAboutLicense> licenses READ licenses CONSTANT)
    Q_PROPERTY(QString copyrightStatement READ copyrightStatement CONSTANT)
    Q_PROPERTY(QString desktopFileName READ desktopFileName CONSTANT)
public:
    /*!
     * Returns the KAboutData for the application.
     *
     * This contains information such as authors, license, etc.,
     * provided that setApplicationData has been called before.
     * If not called before, the returned KAboutData will be initialized from the
     * equivalent properties of QCoreApplication (and its subclasses),
     * if an instance of that already exists.
     * For the list of such properties see setApplicationData
     * (before 5.22: limited to QCoreApplication::applicationName).
     * \sa setApplicationData
     */
    static KAboutData applicationData();

    /*!
     * Sets the application data for this application.
     *
     * In addition to changing the result of applicationData, this initializes
     * the equivalent properties of QCoreApplication (and its subclasses) with
     * information from \a aboutData, if an instance of that already exists.
     * Those properties are:
     *  \list
     *  \li QCoreApplication::applicationName
     *  \li QCoreApplication::applicationVersion
     *  \li QCoreApplication::organizationDomain
     *  \li QGuiApplication::applicationDisplayName
     *  \li QGuiApplication::desktopFileName (since 5.16)
     *  \endlist
     * \sa applicationData
     */
    static void setApplicationData(const KAboutData &aboutData);

public:
    /*!
     * Constructor.
     *
     * Porting Note: The \a catalogName parameter present in KDE4 was
     * deprecated and removed. See also K4AboutData
     * in kde4support if this feature is needed for compatibility purposes, or
     * consider using componentName() instead.
     *
     * \a componentName The program name or plugin name used internally.
     * Example: QStringLiteral("kwrite"). This should never be translated.
     *
     * \a displayName A displayable name for the program or plugin. This string
     *        should be translated. Example: i18n("KWrite")
     *
     * \a version The component version string. Example: QStringLiteral("1.0").
     *
     * \a shortDescription A short description of what the component does.
     *        This string should be translated.
     *        Example: i18n("A simple text editor.")
     *
     * \a licenseType The license identifier. Use setLicenseText or
              setLicenseTextFile if you use a license not predefined here.
     *
     * \a copyrightStatement A copyright statement, that can look like this:
     *        i18n("Copyright (C) 1999-2000 Name"). The string specified here is
     *        taken verbatim; the author information from addAuthor is not used.
     *
     * \a otherText Some free form text, that can contain any kind of
     *        information. The text can contain newlines. This string
     *        should be translated.
     *
     * \a homePageAddress The URL to the component's homepage, including
     *        URL scheme. "http://some.domain" is correct, "some.domain" is
     *        not. Since KDE Frameworks 5.17, https and other valid URL schemes
     *        are also valid. See also the note below.
     *
     * \a bugAddress The bug report address string, an email address or a URL.
     *        This defaults to the kde.org bug system.
     *
     * \note The \a homePageAddress argument is used to derive a default organization
     * domain for the application (which is used to register on the session D-Bus,
     * locate the appropriate desktop file, etc.), by taking the host name and dropping
     * the first component, unless there are less than three (e.g. "www.kde.org" -> "kde.org").
     * Use both setOrganizationDomain(const QByteArray&) and setDesktopFileName() if their default values
     * do not have proper values.
     *
     * \sa setOrganizationDomain(const QByteArray&), setDesktopFileName(const QString&)
     */
    // KF6: remove constructor that includes catalogName, and put default
    //      values back in for shortDescription and licenseType
    KAboutData(const QString &componentName,
               const QString &displayName,
               const QString &version,
               const QString &shortDescription,
               enum KAboutLicense::LicenseKey licenseType,
               const QString &copyrightStatement = QString(),
               const QString &otherText = QString(),
               const QString &homePageAddress = QString(),
               const QString &bugAddress = QStringLiteral("submit@bugs.kde.org"));

    /*!
     * Constructor.
     *
     * \a componentName The program name or plugin name used internally.
     * Example: "kwrite".
     *
     * \a displayName A displayable name for the program or plugin. This string
     *        should be translated. Example: i18n("KWrite")
     *
     * \a version The component version string.
     *
     * Sets the property desktopFileName to "org.kde."+componentName and
     * the property organizationDomain to "kde.org".
     *
     * Default arguments since KCoreAddons 5.53
     *
     * \sa setOrganizationDomain(const QByteArray&), setDesktopFileName(const QString&)
     */
    explicit KAboutData(const QString &componentName = {}, const QString &displayName = {}, const QString &version = {});

    /*!
     * Copy constructor.  Performs a deep copy.
     *
     * \a other object to copy
     */
    KAboutData(const KAboutData &other);

    /*!
     * Assignment operator.  Performs a deep copy.
     *
     * \a other object to copy
     */
    KAboutData &operator=(const KAboutData &other);

    ~KAboutData();

    /*!
     * Defines an author.
     *
     * You can call this function as many times as you need. Each entry is
     * appended to a list. The person in the first entry is assumed to be
     * the leader of the project.
     *
     * \a name The developer's name. It should be translated.
     *
     * \a task What the person is responsible for. This text can contain
     *             newlines. It should be translated.
     *             Can be left empty.
     *
     * \a emailAddress An Email address where the person can be reached.
     *                     Can be left empty.
     *
     * \a webAddress The person's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        correct, "some.domain" is not. Can be left empty.
     *
     * \a avatarUrl URL to the avatar of the person
     */
    KAboutData &addAuthor(const QString &name,
                          const QString &task = QString(),
                          const QString &emailAddress = QString(),
                          const QString &webAddress = QString(),
                          const QUrl &avatarUrl = QUrl());

    /*!
     * \overload
     * \since KCoreAddons 6.0
     */
    KAboutData &addAuthor(const QString &name, const QString &task, const QString &emailAddress, const QString &webAddress, const QString &kdeStoreUsername)
    {
        return addAuthor(name, task, emailAddress, webAddress, QUrl(QStringLiteral("https://store.kde.org/avatar/") + kdeStoreUsername));
    }

    /*!
     * Defines a person that deserves credit.
     *
     * You can call this function as many times as you need. Each entry
     * is appended to a list.
     *
     * \a name The person's name. It should be translated.
     *
     * \a task What the person has done to deserve the honor. The
     *        text can contain newlines. It should be translated.
     *        Can be left empty.
     *
     * \a emailAddress An email address when the person can be reached.
     *        Can be left empty.
     *
     * \a webAddress The person's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        is correct, "some.domain" is not. Can be left empty.
     *
     * \a avatarUrl URL to the avatar of the person
     */
    KAboutData &addCredit(const QString &name,
                          const QString &task = QString(),
                          const QString &emailAddress = QString(),
                          const QString &webAddress = QString(),
                          const QUrl &avatarUrl = QUrl());

    /*!
     * \overload
     * \since KCoreAddons 6.0
     */
    KAboutData &addCredit(const QString &name, const QString &task, const QString &emailAddress, const QString &webAddress, const QString &kdeStoreUsername)
    {
        return addCredit(name, task, emailAddress, webAddress, QUrl(QStringLiteral("https://store.kde.org/avatar/") + kdeStoreUsername));
    }

    /*!
     * \brief Sets the name(s) of the translator(s) of the GUI.
     *
     * The canonical use with the ki18n framework is:
     *
     * \code
     * setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"),
     *               i18nc("EMAIL OF TRANSLATORS", "Your emails"));
     * \endcode
     *
     * If you are using a KMainWindow this is done for you automatically.
     *
     * The name and emailAddress are treated as lists separated with ",".
     *
     * If the strings are empty or "Your names"/"Your emails"
     * respectively they will be ignored.
     *
     * \a name the name(s) of the translator(s)
     *
     * \a emailAddress the email address(es) of the translator(s)
     *
     * \sa KAboutTranslator
     */
    KAboutData &setTranslator(const QString &name, const QString &emailAddress);

    /*!
     * Defines a component that is used by the application.
     *
     * You can call this function as many times as you need. Each entry is
     * appended to a list.
     *
     * \a name The component's name. It should be translated.
     *
     * \a description Short description of the component and maybe
     *        copyright info. This text can contain newlines. It should
     *        be translated. Can be left empty.
     *
     * \a version The version of the component. Can be left empty.
     *
     * \a webAddress The component's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        correct, "some.domain" is not. Can be left empty.
     *
     * \a licenseKey The component's license identifier. Can be left empty (i.e. KAboutLicense::Unknown)
     *
     * \since KCoreAddons 5.84
     */
    KAboutData &addComponent(const QString &name,
                             const QString &description = QString(),
                             const QString &version = QString(),
                             const QString &webAddress = QString(),
                             KAboutLicense::LicenseKey licenseKey = KAboutLicense::Unknown);

    /*!
     * Defines a component that is used by the application with a custom license text file.
     *
     * You can call this function as many times as you need. Each entry is
     * appended to a list.
     *
     * \a name The component's name. It should be translated.
     *
     * \a description Short description of the component and maybe
     *        copyright info. This text can contain newlines. It should
     *        be translated. Can be left empty.
     *
     * \a version The version of the component. Can be left empty.
     *
     * \a webAddress The component's homepage or a relevant link.
     *        Start the address with "http://". "http://some.domain" is
     *        correct, "some.domain" is not. Can be left empty.
     *
     * \a pathToLicenseFile Path to the file in the local filesystem containing the license text.
     *        The file format has to be plain text in an encoding compatible to the local.
     *
     * \since KCoreAddons 5.84
     */
    KAboutData &
    addComponent(const QString &name, const QString &description, const QString &version, const QString &webAddress, const QString &pathToLicenseFile);

    /*!
     * Defines a license text, which is translated.
     *
     * Example:
     * \code
     * setLicenseText( i18n("This is my license") );
     * \endcode
     *
     * \a license The license text.
     */
    KAboutData &setLicenseText(const QString &license);

    /*!
     * Adds a license text, which is translated.
     *
     * If there is only one unknown license set, e.g. by using the default
     * parameter in the constructor, that one is replaced.
     *
     * Example:
     * \code
     * addLicenseText( i18n("This is my license") );
     * \endcode
     *
     * \a license The license text.
     * \sa setLicenseText, addLicense, addLicenseTextFile
     */
    KAboutData &addLicenseText(const QString &license);

    /*!
     * Defines a license text by pointing to a file where it resides.
     * The file format has to be plain text in an encoding compatible to the locale.
     *
     * \a file Path to the file in the local filesystem containing the license text.
     */
    KAboutData &setLicenseTextFile(const QString &file);

    /*!
     * Adds a license text by pointing to a file where it resides.
     * The file format has to be plain text in an encoding compatible to the locale.
     *
     * If there is only one unknown license set, e.g. by using the default
     * parameter in the constructor, that one is replaced.
     *
     * \a file Path to the file in the local filesystem containing the license text.
     * \sa addLicenseText, addLicense, setLicenseTextFile
     */
    KAboutData &addLicenseTextFile(const QString &file);

    /*!
     * Defines the component name used internally.
     *
     * \a componentName The application or plugin name. Example: "kate".
     */
    KAboutData &setComponentName(const QString &componentName);

    /*!
     * Defines the displayable component name string.
     *
     * \a displayName The display name. This string should be
     *        translated.
     *        Example: i18n("Advanced Text Editor").
     */
    KAboutData &setDisplayName(const QString &displayName);

    /*!
     * Defines the program logo.
     *
     * Use this if you need to have an application logo
     * in AboutData other than the application icon.
     *
     * Because KAboutData is a core class it cannot use QImage/QPixmap/QIcon directly,
     * so this is a QVariant that should contain a QImage/QPixmap/QIcon.
     *
     * QIcon should be preferred, to be able to properly handle HiDPI scaling.
     * If a QIcon is provided, it will be used at a typical size of 48x48.
     *
     * \a image logo image.
     * \sa programLogo()
     */
    KAboutData &setProgramLogo(const QVariant &image);

    /*!
     * Defines the program version string.
     *
     * \a version The program version.
     */
    KAboutData &setVersion(const QByteArray &version);

    /*!
     * Defines a short description of what the program does.
     *
     * \a shortDescription The program description. This string should
     *        be translated. Example: i18n("An advanced text
     *        editor with syntax highlighting support.").
     */
    KAboutData &setShortDescription(const QString &shortDescription);

    /*!
     * Defines the license identifier.
     *
     * \a licenseKey The license identifier.
     * \sa addLicenseText, setLicenseText, setLicenseTextFile
     */
    KAboutData &setLicense(KAboutLicense::LicenseKey licenseKey);

    /*!
     * Defines the license identifier.
     *
     * \a licenseKey The license identifier.
     *
     * \a versionRestriction Whether later versions of the license are also allowed.
     *    e.g. licensed under "GPL 2.0 or at your option later versions" would be OrLaterVersions.
     * \sa addLicenseText, setLicenseText, setLicenseTextFile
     *
     * \since KCoreAddons 5.37
     */
    KAboutData &setLicense(KAboutLicense::LicenseKey licenseKey, KAboutLicense::VersionRestriction versionRestriction);

    /*!
     * Adds a license identifier.
     *
     * If there is only one unknown license set, e.g. by using the default
     * parameter in the constructor, that one is replaced.
     *
     * \a licenseKey The license identifier.
     * \sa setLicenseText, addLicenseText, addLicenseTextFile
     */
    KAboutData &addLicense(KAboutLicense::LicenseKey licenseKey);

    /*!
     * Adds a license identifier.
     *
     * If there is only one unknown license set, e.g. by using the default
     * parameter in the constructor, that one is replaced.
     *
     * \a licenseKey The license identifier.
     *
     * \a versionRestriction Whether later versions of the license are also allowed.
     *    e.g. licensed under "GPL 2.0 or at your option later versions" would be OrLaterVersions.
     *
     * \sa setLicenseText, addLicenseText, addLicenseTextFile
     *
     * \since KCoreAddons 5.37
     */
    KAboutData &addLicense(KAboutLicense::LicenseKey licenseKey, KAboutLicense::VersionRestriction versionRestriction);

    /*!
     * Defines the copyright statement to show when displaying the license.
     *
     * \a copyrightStatement A copyright statement, that can look like
     *        this: i18n("Copyright (C) 1999-2000 Name"). The string specified here is
     *        taken verbatim; the author information from addAuthor is not used.
     */
    KAboutData &setCopyrightStatement(const QString &copyrightStatement);

    /*!
     * Defines the additional text to show in the about dialog.
     *
     * \a otherText Some free form text, that can contain any kind of
     *        information. The text can contain newlines. This string
     *        should be translated.
     */
    KAboutData &setOtherText(const QString &otherText);

    /*!
     * Defines the program homepage.
     *
     * \a homepage The program homepage string.
     *        Start the address with "http://". "http://kate.kde.org"
     *        is correct but "kate.kde.org" is not.
     */
    KAboutData &setHomepage(const QString &homepage);

    /*!
     * Defines the address where bug reports should be sent.
     *
     * \a bugAddress The bug report email address or URL.
     *        This defaults to the kde.org bug system.
     */
    KAboutData &setBugAddress(const QByteArray &bugAddress);

    /*!
     * Defines the domain of the organization that wrote this application.
     * The domain is set to kde.org by default, or the domain of the homePageAddress constructor argument,
     * if set.
     *
     * Make sure to call setOrganizationDomain(const QByteArray&) if your product
     * is not developed inside the KDE community.
     *
     * Used e.g. for the registration to D-Bus done by KDBusService
     * from the KDE Frameworks KDBusAddons module.
     *
     * Calling this method has no effect on the value of the desktopFileName property.
     *
     * \note If your program should work as a D-Bus activatable service, the base name
     * of the D-Bus service description file or of the desktop file you install must match
     * the D-Bus "well-known name" for which the program will register.
     * For example, KDBusService will use a name created from the reversed organization domain
     * with the component name attached, so for an organization domain "bar.org" and a
     * component name "foo" the name of an installed D-Bus service file needs to be
     * "org.bar.foo.service" or the name of the installed desktop file "org.bar.foo.desktop"
     * (and the desktopFileName property accordingly set to "org.bar.foo").
     *
     * \a domain the domain name, for instance kde.org, koffice.org, etc.
     *
     * \sa setDesktopFileName(const QString&)
     */
    KAboutData &setOrganizationDomain(const QByteArray &domain);

    /*!
     * Defines the product name which will be used in the KBugReport dialog.
     * By default it's the componentName, but you can overwrite it here to provide
     * support for special components e.g. in the form 'product/component',
     * such as 'kontact/summary'.
     *
     * \a name The name of product
     */
    KAboutData &setProductName(const QByteArray &name);

    /*!
     * Returns the application's internal name.
     * Returns the internal program name.
     */
    QString componentName() const;

    /*!
     * Returns the application's product name, which will be used in KBugReport
     * dialog. By default it returns componentName(), otherwise the one which is set
     * with setProductName()
     *
     * Returns the product name.
     */
    QString productName() const;

    /*!
     * \internal
     * Provided for use by KCrash
     */
    const char *internalProductName() const;

    /*!
     * Returns the translated program name.
     * Returns the program name (translated).
     */
    QString displayName() const;

    /*!
     * Returns the domain name of the organization that wrote this application.
     *
     * \sa setOrganizationDomain(const QByteArray&)
     */
    QString organizationDomain() const;

    /*!
     * \internal
     * Provided for use by KCrash
     */
    const char *internalProgramName() const;

    /*!
     * Returns the program logo image.
     *
     * Because KAboutData is a core class it cannot use QImage/QPixmap/QIcon directly,
     * so this is a QVariant containing a QImage/QPixmap/QIcon.
     *
     * Returns the program logo data, or a null image if there is
     *         no custom application logo defined.
     */
    QVariant programLogo() const;

    /*!
     * Returns the program's version.
     * Returns the version string.
     */
    QString version() const;

    /*!
     * \internal
     * Provided for use by KCrash
     */
    const char *internalVersion() const;

    /*!
     * Returns a short, translated description.
     * Returns the short description (translated). Can be
     *         QString() if not set.
     */
    QString shortDescription() const;

    /*!
     * Returns the application homepage.
     * Returns the application homepage URL. Can be QString() if
     *         not set.
     */
    QString homepage() const;

    /*!
     * Returns the email address or URL for bugs.
     * Returns the address where to report bugs.
     */
    QString bugAddress() const;

    /*!
     * \internal
     * Provided for use by KCrash
     */
    const char *internalBugAddress() const;

    /*!
     * Returns a list of authors.
     * Returns author information (list of persons).
     */
    QList<KAboutPerson> authors() const;

    /*!
     * Returns a list of persons who contributed.
     * Returns credit information (list of persons).
     */
    QList<KAboutPerson> credits() const;

    /*!
     * Returns a list of translators.
     * Returns translators information (list of persons)
     */
    QList<KAboutPerson> translators() const;

    /*!
     * Returns a message about the translation team.
     * Returns a message about the translation team
     */
    static QString aboutTranslationTeam();

    /*!
     * Returns a list of components.
     * Returns component information (list of components).
     * \since KCoreAddons 5.84
     */
    QList<KAboutComponent> components() const;

    /*!
     * Returns a translated, free form text.
     * Returns the free form text (translated). Can be QString() if not set.
     */
    QString otherText() const;

    /*!
     * Returns a list of licenses.
     *
     * Returns licenses information (list of licenses)
     */
    QList<KAboutLicense> licenses() const;

    /*!
     * Returns the copyright statement.
     * Returns the copyright statement. Can be QString() if not set.
     */
    QString copyrightStatement() const;

    /*!
     * Returns the plain text displayed around the list of authors instead
     * of the default message telling users to send bug reports to bugAddress().
     *
     * Returns the plain text displayed around the list of authors instead
     *         of the default message.  Can be QString().
     */
    QString customAuthorPlainText() const;

    /*!
     * Returns the rich text displayed around the list of authors instead
     * of the default message telling users to send bug reports to bugAddress().
     *
     * Returns the rich text displayed around the list of authors instead
     *         of the default message.  Can be QString().
     */
    QString customAuthorRichText() const;

    /*!
     * Returns whether custom text should be displayed around the list of
     * authors.
     *
     * Returns whether custom text should be displayed around the list of
     *         authors.
     */
    bool customAuthorTextEnabled() const;

    /*!
     * Sets the custom text displayed around the list of authors instead
     * of the default message telling users to send bug reports to bugAddress().
     *
     * \a plainText The plain text.
     *
     * \a richText The rich text.
     *
     * Setting both to parameters to QString() will cause no message to be
     * displayed at all.  Call unsetCustomAuthorText() to revert to the default
     * message.
     */
    KAboutData &setCustomAuthorText(const QString &plainText, const QString &richText);

    /*!
     * Clears any custom text displayed around the list of authors and falls
     * back to the default message telling users to send bug reports to
     * bugAddress().
     */
    KAboutData &unsetCustomAuthorText();

    /*!
     * Configures the \a parser command line parser to provide an authors entry with
     * information about the developers of the application and an entry specifying the license.
     *
     * Additionally, it will set the description to the command line parser, will add the help
     * option and if the QApplication has a version set (e.g. via KAboutData::setApplicationData)
     * it will also add the version option.
     *
     * Since 5.16 it also adds an option to set the desktop file name.
     *
     * Returns true if adding the options was successful; otherwise returns false.
     *
     * \sa processCommandLine
     */
    bool setupCommandLine(QCommandLineParser *parser);

    /*!
     * Reads the processed \a parser and sees if any of the arguments are the ones set
     * up from setupCommandLine().
     *
     * \sa setupCommandLine()
     */
    void processCommandLine(QCommandLineParser *parser);

    /*!
     * Sets the base name of the desktop entry for this application.
     *
     * This is the file name, without the full path and without extension,
     * of the desktop entry that represents this application according to
     * the freedesktop desktop entry specification (e.g. "org.kde.foo").
     *
     * A default desktop file name is constructed when the KAboutData
     * object is created, using the reverse domain name of the
     * organizationDomain and the componentName as they are at the time
     * of the KAboutData object creation.
     * Call this method to override that default name. Typically this is
     * done when also setOrganizationDomain or setComponentName
     * need to be called to override the initial values.
     *
     * The desktop file name can also be passed to the application at runtime through
     * the \c desktopfile command line option which is added by setupCommandLine.
     * This is useful if an application supports multiple desktop files with different runtime
     * settings.
     *
     * \a desktopFileName The desktop file name of this application
     *
     * \sa desktopFileName
     * \sa organizationDomain
     * \sa componentName
     * \sa setupCommandLine
     * \since KCoreAddons 5.16
     **/
    KAboutData &setDesktopFileName(const QString &desktopFileName);

    /*!
     * Returns The desktop file name of this application (e.g. "org.kde.foo")
     * \sa setDesktopFileName(const QString&)
     * \since KCoreAddons 5.16
     **/
    QString desktopFileName() const;

private:
    friend void KCrash::defaultCrashHandler(int sig);
    // exported for KCrash, no other users intended
    static const KAboutData *applicationDataPointer();

private:
    std::unique_ptr<class KAboutDataPrivate> const d;
};

#endif
