#
# kcoreaddons_desktop_to_json(target desktopfile)
#
# This macro uses desktoptojson to generate a json file from a plugin
# description in a .desktop file. The generated file can be compiled
# into the plugin using the K_PLUGIN_FACTORY_WITH_JSON (cpp) macro.
#
# Example:
#
#  kcoreaddons_desktop_to_json(plasma_engine_time plasma-dataengine-time.desktop)

function(kcoreaddons_desktop_to_json)
    if(${ARGC} EQUAL 1)
        message(WARNING "Calling kcoreaddons_desktop_to_json with only one argument is deprecated. The code should be changed to pass the target which depends on the json file as first argument.")
        set(desktop ${ARGV0})
        set(target "")
    else()
        set(target ${ARGV0})
        set(desktop ${ARGV1})
    endif()
    string(REPLACE ".desktop" ".json" json ${desktop})

    if(CMAKE_VERSION VERSION_LESS 2.8.12.20140127 OR "${target}" STREQUAL "")
        _desktop_to_json_cmake28(${desktop} ${json})
        return()
    elseif(MSVC_IDE AND CMAKE_VERSION VERSION_LESS 3.0)
        # autogen dependencies for visual studio generator are broken until cmake commit 2ed0d06
        _desktop_to_json_cmake28(${desktop} ${json})
        return()
    endif()

    add_custom_command(
        OUTPUT ${json}
        COMMAND KF5::desktoptojson -i ${desktop} -o ${CMAKE_CURRENT_BINARY_DIR}/${json}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${desktop}
        )
    set_property(TARGET ${target} APPEND PROPERTY AUTOGEN_TARGET_DEPENDS ${json})
endfunction()

function(_desktop_to_json_cmake28 desktop json)
    # This function runs desktoptojson at *configure* time, ie, when CMake runs.
    # This is necessary with CMake < 3.0.0 because the .json file must be
    # generated before moc is run, and there was no way until CMake 3.0.0 to
    # define a target as a dependency of the automoc target.
    message("Using CMake 2.8 way to call desktoptojson")
    get_target_property(DESKTOPTOJSON_LOCATION KF5::desktoptojson LOCATION)
    execute_process(
        COMMAND ${DESKTOPTOJSON_LOCATION} -i ${desktop} -o ${CMAKE_CURRENT_BINARY_DIR}/${json}
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    if (NOT result EQUAL 0)
        message(FATAL_ERROR "Generating ${json} failed")
    endif()
endfunction()
