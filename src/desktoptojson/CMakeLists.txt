include(ECMMarkNonGuiExecutable)

add_executable(desktoptojson)

ecm_qt_declare_logging_category(desktoptojson
    HEADER desktoptojson_debug.h
    # reusing the one also used with desktopfileparser.cpp in library case
    IDENTIFIER DESKTOPPARSER
    CATEGORY_NAME kf.coreaddons.desktoptojson
    # enable debug messages of desktopfileparser.cpp by default, other than in library case
    DEFAULT_SEVERITY Debug
    DESCRIPTION "desktoptojson (KCoreAddons)"
    EXPORT KCOREADDONS
)

target_sources(desktoptojson PRIVATE
    main.cpp
    desktoptojson.cpp
    desktoptojson.h
    ../lib/plugin/desktopfileparser.cpp
)
if(NOT CMAKE_TOOLCHAIN_FILE)
    add_executable(KF5::desktoptojson ALIAS desktoptojson)
endif()

set_property(TARGET desktoptojson PROPERTY CXX_STANDARD 14)

# Mark it as non-gui so we won't create an app bundle on Mac OS X
ecm_mark_nongui_executable(desktoptojson)

target_link_libraries(desktoptojson Qt${QT_MAJOR_VERSION}::Core)
target_compile_definitions(desktoptojson PRIVATE BUILDING_DESKTOPTOJSON_TOOL=1)
# Note: we add a custom TOOL_VERSION property here to enable compatibility checks when cross-compiling.
set_target_properties(desktoptojson PROPERTIES
        TOOL_VERSION "${KF_VERSION}"
        EXPORT_PROPERTIES TOOL_VERSION)

install(TARGETS desktoptojson EXPORT KF5CoreAddonsToolingTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
