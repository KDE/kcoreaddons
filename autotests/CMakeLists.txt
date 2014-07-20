remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)
include(ConfigureChecks.cmake) #configure checks for QFileSystemWatcher

find_package(Qt5Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

macro(build_plugin pname)
    add_library(${pname} MODULE ${ARGN})
    ecm_mark_as_test(${pname})
    target_link_libraries(${pname} KF5::CoreAddons)
endmacro()

# Build some sample plugins
build_plugin(jsonplugin jsonplugin.cpp)
build_plugin(jsonplugin2 jsonplugin2.cpp)
build_plugin(versionedplugin versionedplugin.cpp)
build_plugin(unversionedplugin unversionedplugin.cpp)
build_plugin(multiplugin multiplugin.cpp)
build_plugin(alwaysunloadplugin alwaysunloadplugin.cpp)

ecm_add_tests(
    kaboutdatatest.cpp
    kautosavefiletest.cpp
    kcompositejobtest.cpp
    kformattest.cpp
    kjobtest.cpp
    kpluginfactorytest.cpp
    kpluginloadertest.cpp
    kpluginmetadatatest.cpp
    kprocesstest.cpp
    kshareddatacachetest.cpp
    kshelltest.cpp
    kurlmimedatatest.cpp
    kstringhandlertest.cpp
    kusertest.cpp
    kdelibs4migrationtest.cpp
    LINK_LIBRARIES Qt5::Test KF5::CoreAddons
)

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
    target_link_libraries(${BACKEND_TEST_TARGET} Qt5::Test KF5::CoreAddons)
    ecm_mark_as_test(${BACKEND_TEST_TARGET})
    add_test(NAME ${BACKEND_TEST_TARGET} COMMAND ${BACKEND_TEST_TARGET})
    target_compile_definitions(${BACKEND_TEST_TARGET} PUBLIC -DKDIRWATCH_TEST_METHOD=\"${_backendName}\")
endforeach()
