include(ECMAddTests)
include(ConfigureChecks.cmake) #configure checks for QFileSystemWatcher

find_package(Qt5Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

if(NOT CMAKE_BUILD_TYPE MATCHES "[Dd]ebug$")
    set(ENABLE_BENCHMARKS 1)
endif()

add_library(autotests_static STATIC)
ecm_qt_declare_logging_category(autotests_static
        HEADER kcoreaddons_debug.h
        IDENTIFIER KCOREADDONS_DEBUG
        CATEGORY_NAME kf.coreaddons
        )
target_link_libraries(autotests_static Qt5::Core)

configure_file(config-tests.h.in config-tests.h)

macro(build_plugin pname)
    add_library(${pname} MODULE ${ARGN})
    ecm_mark_as_test(${pname})
    target_link_libraries(${pname} KF5::CoreAddons autotests_static)
endmacro()

# Build some sample plugins
build_plugin(jsonplugin jsonplugin.cpp)
build_plugin(jsonplugin2 jsonplugin2.cpp)
build_plugin(versionedplugin versionedplugin.cpp)
build_plugin(unversionedplugin unversionedplugin.cpp)
build_plugin(multiplugin multiplugin.cpp)
build_plugin(alwaysunloadplugin alwaysunloadplugin.cpp)
build_plugin(qtplugin qtplugin.cpp)

add_definitions( -DKDELIBS4CONFIGMIGRATOR_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )

if (WIN32)
    set(autotests_OPTIONAL_SRCS
        ${autotests_OPTIONAL_SRCS}
        klistopenfilesjobtest_win.cpp
    )
endif ()

if (UNIX)
    set(autotests_OPTIONAL_SRCS
        ${autotests_OPTIONAL_SRCS}
        klistopenfilesjobtest_unix.cpp
    )
endif ()

ecm_add_tests(
    kaboutdatatest.cpp
    kaboutdataapplicationdatatest.cpp
    kautosavefiletest.cpp
    kcompositejobtest.cpp
    kformattest.cpp
    kjobtest.cpp
    kosreleasetest.cpp
    kpluginfactorytest.cpp
    kpluginloadertest.cpp
    kpluginmetadatatest.cpp
    kprocesstest.cpp
    krandomtest.cpp
    kshareddatacachetest.cpp
    kshelltest.cpp
    kurlmimedatatest.cpp
    kstringhandlertest.cpp
    kusertest.cpp
    kdelibs4migrationtest.cpp
    kdelibs4configmigratortest.cpp
    kprocesslisttest.cpp
    kfileutilstest.cpp
    kfuzzymatchertest.cpp
    knetworkmountstestcanonical.cpp
    knetworkmountstestnoconfig.cpp
    knetworkmountstestpaths.cpp
    knetworkmountsteststatic.cpp
    ${autotests_OPTIONAL_SRCS}
    LINK_LIBRARIES Qt5::Test KF5::CoreAddons autotests_static
)

if(NOT CMAKE_CROSSCOMPILING)
    ecm_add_tests(desktoptojsontest.cpp LINK_LIBRARIES Qt5::Test KF5::CoreAddons autotests_static)
    target_compile_definitions(desktoptojsontest PRIVATE
        DESKTOP_TO_JSON_EXE="$<TARGET_FILE:desktoptojson>"
    )
endif()

ecm_add_test(ktexttohtmltest.cpp ${CMAKE_SOURCE_DIR}/src/lib/text/ktexttohtml.cpp
    TEST_NAME ktexttohtmltest
    LINK_LIBRARIES Qt5::Test autotests_static
)
# include the binary dir in order to get kcoreaddons_export.h
target_include_directories(ktexttohtmltest PRIVATE ${KCoreAddons_BINARY_DIR}/src/lib)
# fake static linking to prevent the export macros on Windows from kicking in
target_compile_definitions(ktexttohtmltest PRIVATE -DKCOREADDONS_STATIC_DEFINE=1)

add_executable(kprocesstest_helper kprocesstest_helper.cpp)
target_link_libraries(kprocesstest_helper KF5::CoreAddons)

target_compile_definitions(kpluginloadertest PRIVATE
    JSONPLUGIN_FILE="$<TARGET_FILE:jsonplugin>"
    VERSIONEDPLUGIN_FILE="$<TARGET_FILE:versionedplugin>"
    UNVERSIONEDPLUGIN_FILE="$<TARGET_FILE:unversionedplugin>"
    MULTIPLUGIN_FILE="$<TARGET_FILE:multiplugin>"
    ALWAYSUNLOADPLUGIN_FILE="$<TARGET_FILE:alwaysunloadplugin>"
)

set(KDIRWATCH_BACKENDS_TO_TEST Stat)#Stat is always compiled

if (HAVE_SYS_INOTIFY_H)
    list(APPEND KDIRWATCH_BACKENDS_TO_TEST INotify)
endif()

if (HAVE_FAM)
    list(APPEND KDIRWATCH_BACKENDS_TO_TEST Fam)
endif()

if (HAVE_QFILESYSTEMWATCHER)
    list(APPEND KDIRWATCH_BACKENDS_TO_TEST QFSWatch)
endif()

foreach(_backendName ${KDIRWATCH_BACKENDS_TO_TEST})
    string(TOLOWER ${_backendName} _lowercaseBackendName)
    set(BACKEND_TEST_TARGET kdirwatch_${_lowercaseBackendName}_unittest)
    add_executable(${BACKEND_TEST_TARGET} kdirwatch_unittest.cpp)
    target_link_libraries(${BACKEND_TEST_TARGET} Qt5::Test KF5::CoreAddons autotests_static)

    if(NOT WIN32)
        target_link_libraries(${BACKEND_TEST_TARGET} Threads::Threads)
    endif()

    ecm_mark_as_test(${BACKEND_TEST_TARGET})
    add_test(NAME ${BACKEND_TEST_TARGET} COMMAND ${BACKEND_TEST_TARGET})
    target_compile_definitions(${BACKEND_TEST_TARGET} PUBLIC -DKDIRWATCH_TEST_METHOD=\"${_backendName}\")
endforeach()
