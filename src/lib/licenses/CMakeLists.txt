# Install license files for use by KAboutLicense and KAboutData

install(
    FILES
        BSD GPL_V2 GPL_V3 LGPL_V2 LGPL_V21 LGPL_V3 QPL_V1.0 ARTISTIC
    DESTINATION
        ${KDE_INSTALL_DATADIR}/kf${QT_MAJOR_VERSION}/licenses
)
