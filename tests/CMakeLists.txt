remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS Widgets)
if(NOT Qt5Widgets_FOUND)
    message(STATUS "Qt5Widgets not found, examples will not be built.")
    return()
endif()

add_executable(kdirwatchtest_gui kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui Qt5::Widgets KF5::CoreAddons)

add_executable(faceicontest faceicontest.cpp)
target_link_libraries(faceicontest Qt5::Widgets KF5::CoreAddons)

add_executable(texttohtmltest ktexttohtmltest.cpp)
target_link_libraries(texttohtmltest Qt5::Widgets KF5::CoreAddons)
