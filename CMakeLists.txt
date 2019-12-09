cmake_minimum_required(VERSION 3.5)

set(KF5_VERSION "5.65.0") # handled by release scripts
project(KCoreAddons VERSION ${KF5_VERSION})

include(FeatureSummary)
find_package(ECM 5.65.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(ECMSetupQtPluginMacroNames)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(REQUIRED_QT_VERSION 5.11.0)
find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core)

ecm_setup_qtplugin_macro_names(
    JSON_NONE
        "K_PLUGIN_FACTORY"
    JSON_ARG2
        "K_PLUGIN_FACTORY_WITH_JSON"
        "K_PLUGIN_CLASS_WITH_JSON"
    CONFIG_CODE_VARIABLE
        PACKAGE_SETUP_AUTOMOC_VARIABLES
)

find_package(Threads)

# Configure checks for kdirwatch
find_package(FAM)

set_package_properties(FAM PROPERTIES
     PURPOSE "Provides file alteration notification facilities using a separate service. FAM provides additional support for NFS.")

set(HAVE_FAM ${FAM_FOUND})

option(ENABLE_INOTIFY "Try to use inotify for directory monitoring" ON)
if(ENABLE_INOTIFY)
    # Find libinotify
    find_package(Inotify)
    set_package_properties(Inotify PROPERTIES
        PURPOSE "Filesystem alteration notifications using inotify")
    set(HAVE_SYS_INOTIFY_H ${Inotify_FOUND})
else()
    set(HAVE_SYS_INOTIFY_H FALSE)
endif()

option(ENABLE_PROCSTAT "Try to use libprocstat for process information" ON)
if (ENABLE_PROCSTAT)
    # Find libprocstat
    find_package(Procstat)
    set_package_properties(Procstat PROPERTIES
        PURPOSE "Process information using libprocstat")
    set(HAVE_PROCSTAT ${PROCSTAT_FOUND})
else()
    set(HAVE_PROCSTAT FALSE)
endif()

# Generate io/config-kdirwatch.h
configure_file(src/lib/io/config-kdirwatch.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/lib/io/config-kdirwatch.h)

include(ECMPoQmTools)

ecm_setup_version(PROJECT VARIABLE_PREFIX KCOREADDONS
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5CoreAddonsConfigVersion.cmake"
                        SOVERSION 5)


if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()

kde_enable_exceptions()


add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
add_definitions(-DQT_NO_FOREACH)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5CoreAddons")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5CoreAddons_QCH
        FILE KF5CoreAddonsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5CoreAddonsQchTargets.cmake\")")
endif()

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5CoreAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5CoreAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5CoreAddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF5CoreAddonsConfigVersion.cmake"
               "${CMAKE_CURRENT_SOURCE_DIR}/KF5CoreAddonsMacros.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF5CoreAddonsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5CoreAddonsTargets.cmake NAMESPACE KF5:: )
install(EXPORT KF5CoreAddonsToolingTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5CoreAddonsToolingTargets.cmake NAMESPACE KF5:: )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel )

install(FILES kcoreaddons.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
