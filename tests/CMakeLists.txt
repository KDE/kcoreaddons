remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(QT NAMES Qt5 CONFIG QUIET OPTIONAL_COMPONENTS Widgets)

if(${QT_VERSION_MAJOR} EQUAL 5)
    find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT5_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS Widgets)
endif()

if(${QT_VERSION_MAJOR} EQUAL 6)
    find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT6_VERSION} CONFIG QUIET OPTIONAL_COMPONENTS Widgets)
endif()

if(NOT Qt${QT_VERSION_MAJOR}Widgets_FOUND)
    message(STATUS "QtWidgets not found, examples will not be built.")
    return()
endif()

add_executable(kdirwatchtest_gui kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui Qt::Widgets KF5::CoreAddons)

add_executable(faceicontest faceicontest.cpp)
target_link_libraries(faceicontest Qt::Widgets KF5::CoreAddons)
