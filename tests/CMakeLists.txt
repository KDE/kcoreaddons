remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS Widgets)
if(NOT TARGET Qt${QT_MAJOR_VERSION}::Widgets)
    message(STATUS "Qt${QT_MAJOR_VERSION}Widgets not found, examples will not be built.")
    return()
endif()

add_executable(kdirwatchtest_gui kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui Qt${QT_MAJOR_VERSION}::Widgets KF5::CoreAddons)

add_executable(faceicontest faceicontest.cpp)
target_link_libraries(faceicontest Qt${QT_MAJOR_VERSION}::Widgets KF5::CoreAddons)

add_executable(texttohtmltest ktexttohtmltest.cpp)
target_link_libraries(texttohtmltest Qt${QT_MAJOR_VERSION}::Widgets KF5::CoreAddons)
