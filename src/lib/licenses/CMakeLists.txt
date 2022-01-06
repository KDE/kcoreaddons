# Install license files for use by KAboutLicense and KAboutData

if (QT_MAJOR_VERSION STREQUAL "6")
    set(LICENSE_PATH "kf6")
else()
    set(LICENSE_PATH "kf5")
endif()
install(
    FILES
        BSD GPL_V2 GPL_V3 LGPL_V2 LGPL_V21 LGPL_V3 QPL_V1.0 ARTISTIC
    DESTINATION
        ${KDE_INSTALL_DATADIR}/${LICENSE_PATH}/licenses
)
