include(ECMMarkNonGuiExecutable)

ecm_qt_declare_logging_category(desktoptojson_LOG_SRCS
    HEADER desktoptojson_debug.h
    # reusing the one also used with desktopfileparser.cpp in library case
    IDENTIFIER DESKTOPPARSER
    CATEGORY_NAME kf.coreaddons.desktoptojson
    # enable debug messages of desktopfileparser.cpp by default, other than in library case
    DEFAULT_SEVERITY Debug
    DESCRIPTION "desktoptojson (KCoreAddons)"
    EXPORT KCOREADDONS
)

add_executable(desktoptojson
    main.cpp
    desktoptojson.cpp
    ../lib/plugin/desktopfileparser.cpp
    ${desktoptojson_LOG_SRCS}
)
if(NOT CMAKE_TOOLCHAIN_FILE)
    add_executable(KF5::desktoptojson ALIAS desktoptojson)
endif()

set_property(TARGET desktoptojson PROPERTY CXX_STANDARD 14)

# Mark it as non-gui so we won't create an app bundle on Mac OS X
ecm_mark_nongui_executable(desktoptojson)

target_link_libraries(desktoptojson Qt::Core)
target_compile_definitions(desktoptojson PRIVATE BUILDING_DESKTOPTOJSON_TOOL=1)

install(TARGETS desktoptojson EXPORT KF5CoreAddonsToolingTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
