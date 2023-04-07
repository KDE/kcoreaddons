find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS Widgets)
if(NOT TARGET Qt6::Widgets)
    message(STATUS "Qt6Widgets not found, examples will not be built.")
    return()
endif()

add_executable(kdirwatchtest_gui kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui Qt6::Widgets KF6::CoreAddons)

add_executable(faceicontest faceicontest.cpp)
target_link_libraries(faceicontest Qt6::Widgets KF6::CoreAddons)

add_executable(texttohtmltest ktexttohtmltest.cpp)
target_link_libraries(texttohtmltest Qt6::Widgets KF6::CoreAddons)

if (ENABLE_PCH AND NOT WIN32)
    target_precompile_headers(kdirwatchtest_gui REUSE_FROM KF6CoreAddons)
    target_precompile_headers(faceicontest REUSE_FROM KF6CoreAddons)
    target_precompile_headers(texttohtmltest REUSE_FROM KF6CoreAddons)
endif()
