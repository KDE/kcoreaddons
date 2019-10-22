# Configure checks for the caching subdir
include(CheckIncludeFiles)
check_include_files("sys/types.h;sys/mman.h" HAVE_SYS_MMAN_H)
configure_file(caching/config-caching.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-caching.h)

include(CheckSymbolExists)
check_symbol_exists("getgrouplist" "grp.h" HAVE_GETGROUPLIST)
configure_file(util/config-getgrouplist.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-getgrouplist.h)

set (KDE4_DEFAULT_HOME ".kde${_KDE4_DEFAULT_HOME_POSTFIX}" CACHE STRING "The default KDE home directory" )
configure_file(util/config-kde4home.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kde4home.h)

set (ACCOUNTS_SERVICE_ICON_DIR "/var/lib/AccountsService/icons" CACHE STRING "Accounts Services icon storage directory")
configure_file(util/config-accountsservice.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-accountsservice.h)

ecm_create_qm_loader(kcoreaddons_QM_LOADER kcoreaddons5_qt)

set(kcoreaddons_OPTIONAL_SRCS )
set(kcoreaddons_OPTIONAL_LIBS )

if (FAM_FOUND)
   include_directories(${FAM_INCLUDE_DIR})
   set(kcoreaddons_OPTIONAL_LIBS ${kcoreaddons_OPTIONAL_LIBS} ${FAM_LIBRARIES})
endif ()

if (Inotify_FOUND)
   include_directories(${Inotify_INCLUDE_DIRS})
   set(kcoreaddons_OPTIONAL_LIBS ${kcoreaddons_OPTIONAL_LIBS} ${Inotify_LIBRARIES})
endif ()

if(NOT WIN32)
    set(kcoreaddons_OPTIONAL_SRCS caching/kshareddatacache.cpp)
    set(kcoreaddons_OPTIONAL_LIBS ${kcoreaddons_OPTIONAL_LIBS} ${CMAKE_THREAD_LIBS_INIT})

    set_source_files_properties(caching/kshareddatacache.cpp
        PROPERTIES COMPILE_FLAGS -fexceptions)

else()
    set(kcoreaddons_OPTIONAL_SRCS
        caching/kshareddatacache_win.cpp
    )
endif()

if (WIN32)
    set(kcoreaddons_OPTIONAL_SRCS
        ${kcoreaddons_OPTIONAL_SRCS}
        text/kmacroexpander_win.cpp
        util/klistopenfilesjob_win.cpp
        util/kprocesslist_win.cpp
        util/kshell_win.cpp
        util/kuser_win.cpp
    )
endif ()

if (UNIX)
    set(kcoreaddons_OPTIONAL_SRCS
        ${kcoreaddons_OPTIONAL_SRCS}
        text/kmacroexpander_unix.cpp
        util/klistopenfilesjob_unix.cpp
        util/kuser_unix.cpp
        util/kshell_unix.cpp
    )
    if (HAVE_PROCSTAT)
        set(kcoreaddons_OPTIONAL_SRCS
            ${kcoreaddons_OPTIONAL_SRCS}
            util/kprocesslist_unix_procstat.cpp
        )
    else ()
        set(kcoreaddons_OPTIONAL_SRCS
            ${kcoreaddons_OPTIONAL_SRCS}
            util/kprocesslist_unix.cpp
        )
    endif ()
endif ()

set(libkcoreaddons_SRCS
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
    jobs/kcompositejob.cpp
    jobs/kjob.cpp
    jobs/kjobtrackerinterface.cpp
    jobs/kjobuidelegate.cpp
    plugin/kpluginfactory.cpp
    plugin/kpluginloader.cpp
    plugin/kpluginmetadata.cpp
    plugin/desktopfileparser.cpp
    randomness/krandom.cpp
    randomness/krandomsequence.cpp
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
    ${kcoreaddons_OPTIONAL_SRCS}
    ${kcoreaddons_QM_LOADER}
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

if (HAVE_PROCSTAT)
    set(kcoreaddons_INCLUDE_DIRS
        ${kcoreaddons_INCLUDE_DIRS}
        ${PROCSTAT_INCLUDE_DIR}
    )
endif()

ecm_qt_declare_logging_category(libkcoreaddons_SRCS
                                HEADER kcoreaddons_debug.h
                                IDENTIFIER KCOREADDONS_DEBUG
                                CATEGORY_NAME org.kde.kcoreaddons)

add_library(KF5CoreAddons ${libkcoreaddons_SRCS})
add_library(KF5::CoreAddons ALIAS KF5CoreAddons)
ecm_generate_export_header(KF5CoreAddons
    BASE_NAME KCoreAddons
    GROUP_BASE_NAME KF
    VERSION ${KF5_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.0 5.0 5.2
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF5CoreAddons PUBLIC "$<BUILD_INTERFACE:${kcoreaddons_INCLUDE_DIRS}>")

target_link_libraries(KF5CoreAddons
  PUBLIC
    Qt5::Core
  PRIVATE
    ${kcoreaddons_OPTIONAL_LIBS}
)

target_link_libraries(KF5CoreAddons PRIVATE ${CMAKE_THREAD_LIBS_INIT})

if(WIN32)
    target_link_libraries(KF5CoreAddons PRIVATE netapi32 userenv)
endif()

if(HAVE_PROCSTAT)
    target_link_libraries(KF5CoreAddons PRIVATE ${PROCSTAT_LIBRARIES})
endif()

target_include_directories(KF5CoreAddons INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KCoreAddons>" )

target_compile_definitions(KF5CoreAddons INTERFACE "$<INSTALL_INTERFACE:KCOREADDONS_LIB>")

set_target_properties(KF5CoreAddons PROPERTIES VERSION   ${KCOREADDONS_VERSION_STRING}
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
    RELATIVE util
    REQUIRED_HEADERS KCoreAddons_HEADERS
)

find_package(PythonModuleGeneration)

if (PythonModuleGeneration_FOUND)
  ecm_generate_python_binding(
    TARGET KF5::CoreAddons
    PYTHONNAMESPACE PyKF5
    MODULENAME KCoreAddons
    RULES_FILE "${CMAKE_SOURCE_DIR}/cmake/rules_PyKF5.py"
    SIP_DEPENDS
      QtCore/QtCoremod.sip
    HEADERS
      kaboutdata.h
      kcoreaddons.h
      caching/kshareddatacache.h
      io/kautosavefile.h
      io/kdirwatch.h
      io/kmessage.h
      io/kprocess.h
      io/kbackup.h
      io/kurlmimedata.h
      io/kfilesystemtype.h
      jobs/kcompositejob.h
      jobs/kjob.h
      jobs/kjobtrackerinterface.h
      jobs/kjobuidelegate.h
      plugin/kexportplugin.h
      plugin/kpluginfactory.h
      plugin/kpluginloader.h
      plugin/kpluginmetadata.h
      randomness/krandom.h
      randomness/krandomsequence.h
      text/kmacroexpander.h
      text/kstringhandler.h
      text/ktexttohtml.h
      text/ktexttohtmlemoticonsinterface.h
      util/kformat.h
      util/klistopenfilesjob.h
      util/kosrelease.h
      util/kprocesslist.h
      util/kuser.h
      util/kshell.h
      util/kdelibs4migration.h
      util/kdelibs4configmigrator.h
  )
endif()

install(TARGETS KF5CoreAddons EXPORT KF5CoreAddonsTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${KCoreAddons_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KCoreAddons COMPONENT Devel
)

# Includes CMake code to install open-source license texts for KAboutData.
add_subdirectory(licenses)

if(BUILD_QCH)
    ecm_add_qch(
        KF5CoreAddons_QCH
        NAME KCoreAddons
        BASE_NAME KF5CoreAddons
        VERSION ${KF5_VERSION}
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
ecm_generate_pri_file(BASE_NAME KCoreAddons LIB_NAME KF5CoreAddons DEPS "core" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KCoreAddons)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
