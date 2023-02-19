# Configure checks for the caching subdir
include(CheckIncludeFiles)
check_include_files("sys/types.h;sys/mman.h" HAVE_SYS_MMAN_H)
configure_file(caching/config-caching.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-caching.h)

include(CheckSymbolExists)
check_symbol_exists("getgrouplist" "grp.h" HAVE_GETGROUPLIST)

if(UNIX)
    function(check_dladdr) # use a function to scope the variables!
        set(CMAKE_REQUIRED_FLAGS ${DLADDR_LINK_FLAGS})
        set(CMAKE_REQUIRED_LIBRARIES ${DLADDR_LINK_LIBRARIES})
        check_symbol_exists("dladdr" "dlfcn.h" HAVE_DLADDR)
        set(HAVE_DLADDR ${HAVE_DLADDR} PARENT_SCOPE)
    endfunction()

    if(CMAKE_SYSTEM_NAME MATCHES "Linux") # on linux dladdr isn't part of libc, on freebsd and osx it is
        set(DLADDR_LINK_FLAGS "-D_GNU_SOURCE")
        set(DLADDR_LINK_LIBRARIES "dl")
    endif()
    check_dladdr()

    add_feature_info(dladdr ${HAVE_DLADDR} "Can resolve shared library paths and by extension libexec paths at runtime using dladdr() API")
endif()

set (KDE4_DEFAULT_HOME ".kde${_KDE4_DEFAULT_HOME_POSTFIX}" CACHE STRING "The default KDE home directory" )
set (ACCOUNTS_SERVICE_ICON_DIR "/var/lib/AccountsService/icons" CACHE STRING "Accounts Services icon storage directory")

configure_file(util/config-util.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-util.h)

add_library(KF5CoreAddons)
add_library(KF5::CoreAddons ALIAS KF5CoreAddons)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(KF5CoreAddons PUBLIC -DKCOREADDONS_STATIC)
endif()

ecm_create_qm_loader(KF5CoreAddons kcoreaddons5_qt)

if (FAM_FOUND)
    target_include_directories(KF5CoreAddons PRIVATE ${FAM_INCLUDE_DIR})
    target_link_libraries(KF5CoreAddons PRIVATE ${FAM_LIBRARIES})
endif ()

if (Inotify_FOUND)
    target_include_directories(KF5CoreAddons PRIVATE ${Inotify_INCLUDE_DIRS})
    target_link_libraries(KF5CoreAddons PRIVATE ${Inotify_LIBRARIES})
endif ()

if(NOT WIN32)
    target_sources(KF5CoreAddons PRIVATE
        caching/kshareddatacache.cpp
        caching/kshareddatacache.h
    )

    set_source_files_properties(caching/kshareddatacache.cpp
        PROPERTIES COMPILE_FLAGS -fexceptions)

    target_link_libraries(KF5CoreAddons PRIVATE Threads::Threads)
else()
    target_sources(KF5CoreAddons PRIVATE
        caching/kshareddatacache_win.cpp
    )
endif()

if (WIN32)
    target_sources(KF5CoreAddons PRIVATE
        text/kmacroexpander_win.cpp
        util/klistopenfilesjob_win.cpp
        util/kprocesslist_win.cpp
        util/kshell_win.cpp
        util/kuser_win.cpp
    )
endif ()

if (UNIX)
    target_sources(KF5CoreAddons PRIVATE
        text/kmacroexpander_unix.cpp
        util/klistopenfilesjob_unix.cpp
        util/kuser_unix.cpp
        util/kshell_unix.cpp
    )
    if (HAVE_PROCSTAT)
        target_sources(KF5CoreAddons PRIVATE
            util/kprocesslist_unix_procstat.cpp
        )
        target_compile_definitions(KF5CoreAddons PRIVATE HAVE_PROCSTAT=1)
    else ()
        target_sources(KF5CoreAddons PRIVATE
            util/kprocesslist_unix.cpp
        )
    endif ()
endif ()

if(HAVE_QTDBUS)
    set(_dbus_SRCS)

    set_source_files_properties(io/org.freedesktop.portal.FileTransfer.xml PROPERTIES INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/io/dbustypes_p.h)
    qt_add_dbus_interface(_dbus_SRCS io/org.freedesktop.portal.FileTransfer.xml io/org.freedesktop.portal.FileTransfer)

    set_source_files_properties(io/org.kde.KIOFuse.VFS.xml PROPERTIES NO_NAMESPACE TRUE)
    qt_add_dbus_interface(_dbus_SRCS io/org.kde.KIOFuse.VFS.xml io/org.kde.KIOFuse.VFS)

    target_sources(KF5CoreAddons PRIVATE ${_dbus_SRCS})
    target_link_libraries(KF5CoreAddons PRIVATE Qt${QT_MAJOR_VERSION}::DBus)
endif()

if (TARGET UDev::UDev)
    target_link_libraries(KF5CoreAddons PRIVATE UDev::UDev)
endif()

target_sources(KF5CoreAddons PRIVATE
    kaboutdata.cpp
    kcoreaddons.cpp
    io/kautosavefile.cpp
    io/kdirwatch.cpp
    io/kfilesystemtype.cpp
    io/kmessage.cpp
    io/kprocess.cpp
    io/kbackup.cpp
    io/kurlmimedata.cpp
    io/kfileutils.cpp
    io/knetworkmounts.cpp
    jobs/kcompositejob.cpp
    jobs/kjob.cpp
    jobs/kjobtrackerinterface.cpp
    jobs/kjobuidelegate.cpp
    plugin/kpluginfactory.cpp
    plugin/kpluginloader.cpp
    plugin/kpluginmetadata.cpp
    plugin/kstaticpluginhelpers.cpp
    plugin/desktopfileparser.cpp
    randomness/krandom.cpp
    randomness/krandomsequence.cpp
    text/kjsonutils.cpp
    text/kfuzzymatcher.cpp
    text/kmacroexpander.cpp
    text/kstringhandler.cpp
    text/ktexttohtml.cpp
    util/kdelibs4migration.cpp
    util/kdelibs4configmigrator.cpp
    util/kformat.cpp
    util/kformatprivate.cpp
    util/kosrelease.cpp
    util/kprocesslist.cpp
    util/kshell.cpp
    util/klibexec.cpp
    util/ksignalhandler.cpp
    util/ksandbox.cpp
    util/kmemoryinfo.cpp
    util/kruntimeplatform.cpp

    kaboutdata.h
    kcoreaddons.h
    io/kautosavefile.h
    io/kdirwatch.h
    io/kfilesystemtype.h
    io/kmessage.h
    io/kprocess.h
    io/kbackup.h
    io/kurlmimedata.h
    io/kfileutils.h
    io/knetworkmounts.h
    jobs/kcompositejob.h
    jobs/kjob.h
    jobs/kjobtrackerinterface.h
    jobs/kjobuidelegate.h
    plugin/kpluginfactory.h
    plugin/kpluginloader.h
    plugin/kpluginmetadata.h
    plugin/kstaticpluginhelpers.h
    randomness/krandom.h
    randomness/krandomsequence.h
    text/kjsonutils.h
    text/kfuzzymatcher.h
    text/kmacroexpander.h
    text/kstringhandler.h
    text/ktexttohtml.h
    util/kdelibs4migration.h
    util/kdelibs4configmigrator.h
    util/kformat.h
    util/kosrelease.h
    util/kprocesslist.h
    util/kshell.h
    util/klibexec.h
    util/ksignalhandler.h
    util/ksandbox.h
    util/kmemoryinfo.h
    util/kruntimeplatform.h
)


set(kcoreaddons_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}/../.. # for kcoreaddons_version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/caching/
    ${CMAKE_CURRENT_BINARY_DIR}/io/
    ${CMAKE_CURRENT_SOURCE_DIR}/io/
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin/
    ${CMAKE_CURRENT_SOURCE_DIR}/randomness/
    ${CMAKE_CURRENT_SOURCE_DIR}/text/
    ${CMAKE_CURRENT_SOURCE_DIR}/util/
)

ecm_qt_export_logging_category(
    IDENTIFIER KDIRWATCH
    CATEGORY_NAME kf.coreaddons.kdirwatch
    OLD_CATEGORY_NAMES kf5.kcoreaddons.kdirwatch
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KDirWatch (KCoreAddons)"
    EXPORT KCOREADDONS
)
ecm_qt_export_logging_category(
    IDENTIFIER KABOUTDATA
    CATEGORY_NAME kf.coreaddons.kaboutdata
    OLD_CATEGORY_NAMES kf5.kcoreaddons.kaboutdata
    DESCRIPTION "KAboutData (KCoreAddons)"
    EXPORT KCOREADDONS
)
ecm_qt_export_logging_category(
    IDENTIFIER LOG_KMEMORYINFO
    CATEGORY_NAME kf.coreaddons.kmemoryinfo
    DESCRIPTION "KMemoryInfo (KCoreAddons)"
    EXPORT KCOREADDONS
)
ecm_qt_declare_logging_category(KF5CoreAddons
    HEADER desktopfileparser_debug.h
    IDENTIFIER DESKTOPPARSER
    CATEGORY_NAME kf.coreaddons.desktopparser
    OLD_CATEGORY_NAMES kf5.kcoreaddons.desktopparser
    DEFAULT_SEVERITY Warning
    DESCRIPTION "DesktopParser (KCoreAddons)"
    EXPORT KCOREADDONS
)
ecm_qt_export_logging_category(
    IDENTIFIER MIGRATOR
    CATEGORY_NAME kf.coreaddons.kdelibs4configmigrator
    OLD_CATEGORY_NAMES kf5.kcoreaddons.kdelibs4configmigrator
    DEFAULT_SEVERITY Warning
    DESCRIPTION "Kdelibs4ConfigMigrator (KCoreAddons)"
    EXPORT KCOREADDONS
)

ecm_qt_declare_logging_category(KF5CoreAddons
    HEADER kcoreaddons_debug.h
    IDENTIFIER KCOREADDONS_DEBUG
    CATEGORY_NAME kf.coreaddons
    OLD_CATEGORY_NAMES org.kde.kcoreaddons
    DESCRIPTION "kcoreaddons (kcoreaddons lib)"
    EXPORT KCOREADDONS
)

ecm_generate_export_header(KF5CoreAddons
    BASE_NAME KCoreAddons
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.0 5.0 5.2 5.65 5.67 5.70 5.72 5.75 5.76 5.78 5.79 5.80 5.84 5.86 5.87 5.88 5.89 5.92 5.95 5.97 5.104
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF5CoreAddons PUBLIC "$<BUILD_INTERFACE:${kcoreaddons_INCLUDE_DIRS}>")

target_link_libraries(KF5CoreAddons
  PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
)

if(WIN32)
    target_link_libraries(KF5CoreAddons PRIVATE netapi32 userenv psapi)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    target_link_libraries(KF5CoreAddons PRIVATE kvm)
endif()

if (HAVE_PROCSTAT)
    target_link_libraries(KF5CoreAddons PRIVATE Procstat::Procstat)
endif()

if(HAVE_DLADDR)
    set_source_files_properties(SOURCE util/klibexec.cpp PROPERTIES COMPILE_FLAGS "${DLADDR_LINK_FLAGS}") # ensure _GNU_SOURCE on Linux
    target_link_libraries(KF5CoreAddons PRIVATE ${DLADDR_LINK_LIBRARIES})
endif()

target_include_directories(KF5CoreAddons INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KCoreAddons>" )

target_compile_definitions(KF5CoreAddons INTERFACE "$<INSTALL_INTERFACE:KCOREADDONS_LIB>")

set_target_properties(KF5CoreAddons PROPERTIES VERSION   ${KCOREADDONS_VERSION}
                                               SOVERSION ${KCOREADDONS_SOVERSION}
                                               EXPORT_NAME CoreAddons
)

ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KAboutData
        KCoreAddons
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES KSharedDataCache
    RELATIVE caching
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KAutoSaveFile
        KDirWatch
        KMessage
        KProcess
        KBackup
        KUrlMimeData
        KFileSystemType
        KFileUtils
        KNetworkMounts
    RELATIVE io
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KCompositeJob
        KJob
        KJobTrackerInterface
        KJobUiDelegate
    RELATIVE jobs
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KExportPlugin
        KPluginFactory
        KPluginLoader
        KPluginMetaData
        KStaticPluginHelpers
    RELATIVE plugin
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KRandom
        KRandomSequence
    RELATIVE randomness
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KJsonUtils
        KFuzzyMatcher
        KMacroExpander
        KStringHandler
        KTextToHTML
        KTextToHTMLEmoticonsInterface
    RELATIVE text
    REQUIRED_HEADERS KCoreAddons_HEADERS
)
ecm_generate_headers(KCoreAddons_HEADERS
    HEADER_NAMES
        KFormat
        KOSRelease
        KUser
        KShell
        KProcessList
        KListOpenFilesJob
        Kdelibs4Migration
        Kdelibs4ConfigMigrator
        KLibexec
        KSignalHandler
        KRuntimePlatform
        KSandbox
        KMemoryInfo
    RELATIVE util
    REQUIRED_HEADERS KCoreAddons_HEADERS
)

install(TARGETS KF5CoreAddons EXPORT KF5CoreAddonsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${KCoreAddons_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCoreAddons COMPONENT Devel
)

# Includes CMake code to install open-source license texts for KAboutData.
add_subdirectory(licenses)

if(BUILD_QCH)
    ecm_add_qch(
        KF5CoreAddons_QCH
        NAME KCoreAddons
        BASE_NAME KF5CoreAddons
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KCoreAddons_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
            ${kcoreaddons_INCLUDE_DIRS}
        BLANK_MACROS
            KCOREADDONS_EXPORT
            KCOREADDONS_DEPRECATED
            KCOREADDONS_DEPRECATED_EXPORT
            "KCOREADDONS_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KCoreAddons LIB_NAME KF5CoreAddons DEPS "core" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KCoreAddons)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
