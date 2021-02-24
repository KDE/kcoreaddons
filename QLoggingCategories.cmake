ecm_declare_qloggingcategory(
    IDENTIFIER KCOREADDONS_DEBUG
    CATEGORY_NAME kf.coreaddons
    OLD_CATEGORY_NAMES org.kde.kcoreaddons
    DESCRIPTION "kcoreaddons (kcoreaddons lib)"
)
ecm_declare_qloggingcategory(
    # reusing the one also used with desktopfileparser.cpp in library case
    IDENTIFIER DESKTOPPARSER
    CATEGORY_NAME kf.coreaddons.desktoptojson
    # enable debug messages of desktopfileparser.cpp by default, other than in library case
    DEFAULT_SEVERITY Debug
    DESCRIPTION "desktoptojson (KCoreAddons)"
)
ecm_declare_qloggingcategory(
    IDENTIFIER KDIRWATCH
    CATEGORY_NAME kf.coreaddons.kdirwatch
    OLD_CATEGORY_NAMES kf5.kcoreaddons.kdirwatch
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KDirWatch (KCoreAddons)"
)
ecm_declare_qloggingcategory(
    IDENTIFIER KABOUTDATA
    CATEGORY_NAME kf.coreaddons.kaboutdata
    OLD_CATEGORY_NAMES kf5.kcoreaddons.kaboutdata
    DESCRIPTION "KAboutData (KCoreAddons)"
)
ecm_declare_qloggingcategory(
    IDENTIFIER DESKTOPPARSER
    CATEGORY_NAME kf.coreaddons.desktopparser
    OLD_CATEGORY_NAMES kf5.kcoreaddons.desktopparser
    DEFAULT_SEVERITY Warning
    DESCRIPTION "DesktopParser (KCoreAddons)"
)
ecm_declare_qloggingcategory(
    IDENTIFIER MIGRATOR
    CATEGORY_NAME kf.coreaddons.kdelibs4configmigrator
    OLD_CATEGORY_NAMES kf5.kcoreaddons.kdelibs4configmigrator
    DEFAULT_SEVERITY Warning
    DESCRIPTION "Kdelibs4ConfigMigrator (KCoreAddons)"
)
