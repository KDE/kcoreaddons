remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt5Widgets ${REQUIRED_QT_VERSION} CONFIG QUIET)
if(NOT Qt5Widgets_FOUND)
    message(STATUS "Qt5Widgets not found, examples will not be built.")
    return()
endif()

macro(KCOREADDONS_EXAMPLES)
    foreach(_testname ${ARGN})
        add_executable(${_testname} ${_testname}.cpp) # TODO NOGUI
        target_link_libraries(${_testname} Qt5::Test KF5::CoreAddons)
    endforeach()
endmacro()

########### kdirwatchtest_gui ###############

add_executable(kdirwatchtest_gui kdirwatchtest_gui.cpp)
target_link_libraries(kdirwatchtest_gui Qt5::Widgets KF5::CoreAddons)
