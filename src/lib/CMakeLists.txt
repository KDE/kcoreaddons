# Configure checks for the caching subdir
include(CheckIncludeFiles)
check_include_files("sys/types.h;sys/mman.h" HAVE_SYS_MMAN_H)
configure_file(caching/config-caching.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-caching.h)

set(kcoreaddons_OPTIONAL_SRCS )
set(kcoreaddons_OPTIONAL_LIBS )

if (FAM_FOUND)
   include_directories(${FAM_INCLUDE_DIR})
   set(kcoreaddons_OPTIONAL_LIBS ${kcoreaddons_OPTIONAL_LIBS} ${FAM_LIBRARIES})
endif ()

if(NOT WIN32)
    set(kcoreaddons_OPTIONAL_SRCS caching/kshareddatacache.cpp)
    set(kcoreaddons_OPTIONAL_LIBS ${kcoreaddons_OPTIONAL_LIBS} ${CMAKE_THREAD_LIBS_INIT})

    set_source_files_properties(caching/kshareddatacache.cpp
        PROPERTIES COMPILE_FLAGS -fexceptions)

else()
    set(kcoreaddons_OPTIONAL_SRCS
        caching/kshareddatacache_win.cpp
        io/kdirwatch_win.cpp
    )
endif()

if (WIN32)
    set(kcoreaddons_OPTIONAL_SRCS
        ${kcoreaddons_OPTIONAL_SRCS}
        text/kmacroexpander_win.cpp
        util/kshell_win.cpp
        util/kuser_win.cpp
    )
endif ()

if (UNIX)
    set(kcoreaddons_OPTIONAL_SRCS
        ${kcoreaddons_OPTIONAL_SRCS}
        text/kmacroexpander_unix.cpp
        util/kuser_unix.cpp
        util/kshell_unix.cpp
    )
endif ()

set(libkcoreaddons_SRCS
    kaboutdata.cpp
    io/kautosavefile.cpp
    io/kdirwatch.cpp
    io/kfilesystemtype_p.cpp
    io/kmessage.cpp
    io/kprocess.cpp
    io/kbackup.cpp
    io/kurlmimedata.cpp
    jobs/kcompositejob.cpp
    jobs/kjob.cpp
    jobs/kjobtrackerinterface.cpp
    jobs/kjobuidelegate.cpp
    randomness/krandom.cpp
    randomness/krandomsequence.cpp
    text/kmacroexpander.cpp
    text/kstringhandler.cpp
    util/kformat.cpp
    util/kformatprivate.cpp
    util/kshell.cpp
    ${kcoreaddons_OPTIONAL_SRCS}
)


set(kcoreaddons_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}/../.. # for kcoreaddons_version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/caching/
    ${CMAKE_CURRENT_BINARY_DIR}/io/
    ${CMAKE_CURRENT_SOURCE_DIR}/io/
    ${CMAKE_CURRENT_SOURCE_DIR}/jobs/
    ${CMAKE_CURRENT_SOURCE_DIR}/randomness/
    ${CMAKE_CURRENT_SOURCE_DIR}/text/
    ${CMAKE_CURRENT_SOURCE_DIR}/util/
)

add_library(KF5CoreAddons ${libkcoreaddons_SRCS})
generate_export_header(KF5CoreAddons BASE_NAME KCoreAddons)
add_library(KF5::CoreAddons ALIAS KF5CoreAddons)

target_include_directories(KF5CoreAddons PUBLIC "$<BUILD_INTERFACE:${kcoreaddons_INCLUDE_DIRS}>")

target_link_libraries(KF5CoreAddons
  PUBLIC
    Qt5::Core
  PRIVATE
    ${kcoreaddons_OPTIONAL_LIBS}
)
if(NOT WIN32)
    target_link_libraries(KF5CoreAddons PRIVATE pthread)
else()
    target_link_libraries(KF5CoreAddons PRIVATE netapi32)
endif()

target_include_directories(KF5CoreAddons INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/kcoreaddons;${INCLUDE_INSTALL_DIR}/KCoreAddons>" )

target_compile_definitions(KF5CoreAddons INTERFACE "$<INSTALL_INTERFACE:KCOREADDONS_LIB>")

set_target_properties(KF5CoreAddons PROPERTIES VERSION   ${KCOREADDONS_VERSION_STRING}
                                               SOVERSION ${KCOREADDONS_SOVERSION}
                                               EXPORT_NAME CoreAddons
)

ecm_generate_headers(KAboutData REQUIRED_HEADERS KCoreAddons_HEADERS)
ecm_generate_headers(KSharedDataCache REQUIRED_HEADERS KCoreAddons_HEADERS RELATIVE caching)
ecm_generate_headers(KAutoSaveFile KDirWatch KMessage KProcess KBackup KUrlMimeData
    RELATIVE io REQUIRED_HEADERS KCoreAddons_HEADERS)
ecm_generate_headers(KCompositeJob KJob KJobTrackerInterface KJobUiDelegate
    RELATIVE jobs REQUIRED_HEADERS KCoreAddons_HEADERS)
ecm_generate_headers(KRandom KRandomSequence
    RELATIVE randomness REQUIRED_HEADERS KCoreAddons_HEADERS)
ecm_generate_headers(KMacroExpander KStringHandler
    RELATIVE text REQUIRED_HEADERS KCoreAddons_HEADERS)
ecm_generate_headers(KFormat KUser KShell
    RELATIVE util REQUIRED_HEADERS KCoreAddons_HEADERS)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/KCoreAddons DESTINATION  ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

install(TARGETS KF5CoreAddons EXPORT KF5CoreAddonsTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${KCoreAddons_HEADERS}
    io/kfilesystemtype_p.h #Needed for building kio, KFileSystemType
    ${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_export.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kcoreaddons COMPONENT Devel
)
