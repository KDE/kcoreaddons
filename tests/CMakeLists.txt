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

# With GCC, -Winvalid-pch warnings are there due to missing defines
# so just limit this to clang
if (ENABLE_PCH AND NOT WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_precompile_headers(kdirwatchtest_gui REUSE_FROM KF6CoreAddons)
    target_precompile_headers(faceicontest REUSE_FROM KF6CoreAddons)
    target_precompile_headers(texttohtmltest REUSE_FROM KF6CoreAddons)
endif()
