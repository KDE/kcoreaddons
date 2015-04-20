#
# kcoreaddons_desktop_to_json(target desktopfile [OUTPUT_DIR dir] [COMPAT_MODE])
#
# This macro uses desktoptojson to generate a json file from a plugin
# description in a .desktop file. The generated file can be compiled
# into the plugin using the K_PLUGIN_FACTORY_WITH_JSON (cpp) macro.
#
# If COMPAT_MODE is passed as an argument the generated JSON file will be compatible with
# the metadata format used by KPluginInfo (from KService), otherwise it will default to
# the new format that is used by KPluginMetaData (from KCoreAddons)
#
# If OUTPUT_DIR is set the generated file will be created inside <dir> instead of in
# ${CMAKE_CURRENT_BINARY_DIR}
#
# Example:
#
#  kcoreaddons_desktop_to_json(plasma_engine_time plasma-dataengine-time.desktop)

function(kcoreaddons_desktop_to_json target desktop)
    get_filename_component(desktop_basename ${desktop} NAME_WE) # allow passing an absolute path to the .desktop
    cmake_parse_arguments(DESKTOP_TO_JSON "COMPAT_MODE" "OUTPUT_DIR" "" ${ARGN} )

    if(DESKTOP_TO_JSON_OUTPUT_DIR)
        set(json "${DESKTOP_TO_JSON_OUTPUT_DIR}/${desktop_basename}.json")
    else()
        set(json "${CMAKE_CURRENT_BINARY_DIR}/${desktop_basename}.json")
    endif()

    if(CMAKE_VERSION VERSION_LESS 2.8.12.20140127 OR "${target}" STREQUAL "")
        _desktop_to_json_cmake28(${desktop} ${json} ${DESKTOP_TO_JSON_COMPAT_MODE})
        return()
    elseif(MSVC_IDE AND CMAKE_VERSION VERSION_LESS 3.0)
        # autogen dependencies for visual studio generator are broken until cmake commit 2ed0d06
        _desktop_to_json_cmake28(${desktop} ${json} ${DESKTOP_TO_JSON_COMPAT_MODE})
        return()
    endif()

    if(DESKTOP_TO_JSON_COMPAT_MODE)
        add_custom_command(
            OUTPUT ${json}
            COMMAND KF5::desktoptojson -i ${desktop} -o ${json} -c
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${desktop}
        )
    else()
        add_custom_command(
            OUTPUT ${json}
            COMMAND KF5::desktoptojson -i ${desktop} -o ${json}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${desktop}
        )
    endif()
    set_property(TARGET ${target} APPEND PROPERTY AUTOGEN_TARGET_DEPENDS ${json})
endfunction()

function(_desktop_to_json_cmake28 desktop json compat)
    # This function runs desktoptojson at *configure* time, ie, when CMake runs.
    # This is necessary with CMake < 3.0.0 because the .json file must be
    # generated before moc is run, and there was no way until CMake 3.0.0 to
    # define a target as a dependency of the automoc target.
    message("Using CMake 2.8 way to call desktoptojson")
    get_target_property(DESKTOPTOJSON_LOCATION KF5::desktoptojson LOCATION)
    if(compat)
        execute_process(
            COMMAND ${DESKTOPTOJSON_LOCATION} -i ${desktop} -o ${json} -c
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    else()
        execute_process(
            COMMAND ${DESKTOPTOJSON_LOCATION} -i ${desktop} -o ${json}
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()

    if (NOT result EQUAL 0)
        message(FATAL_ERROR "Generating ${json} failed")
    endif()
endfunction()

#
# kcoreaddons_add_plugin(plugin_name SOURCES... [JSON "pluginname.json"] [INSTALL_NAMESPACE "servicename"])
#
# This macro helps simplifying the creation of plugins for KPluginFactory
# based systems.
# It will create a plugin given the SOURCES list, the name of the JSON file
# that will define the plugin's metadata and the INSTALL_NAMESPACE so that
# the plugin is installed with the rest of the plugins from the same sub-system,
# within ${PLUGIN_INSTALL_DIR}.
#
# Example:
#   kcoreaddons_add_plugin(kdeconnect_share JSON kdeconnect_share.json SOURCES ${kdeconnect_share_SRCS})

function(kcoreaddons_add_plugin plugin)
    set(options)
    set(oneValueArgs JSON INSTALL_NAMESPACE)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(KCA_ADD_PLUGIN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_filename_component(json "${KCA_ADD_PLUGIN_JSON}" REALPATH)

    # ensure we recompile the corresponding object files when the json file changes
    set(dependent_sources )
    foreach(source ${KCA_ADD_PLUGIN_SOURCES})
        get_filename_component(source "${source}" REALPATH)
        if(EXISTS "${source}")
            file(STRINGS "${source}" match REGEX "K_PLUGIN_FACTORY_WITH_JSON")
            if(match)
                list(APPEND dependent_sources "${source}")
            endif()
        endif()
    endforeach()
    if(NOT dependent_sources)
        # fallback to all sources - better safe than sorry...
        set(dependent_sources ${KCA_ADD_PLUGIN_SOURCES})
    endif()
    set_property(SOURCE ${dependent_sources} APPEND PROPERTY OBJECT_DEPENDS ${json})

    add_library(${plugin} MODULE ${KCA_ADD_PLUGIN_SOURCES})
    set_property(TARGET ${plugin} APPEND PROPERTY AUTOGEN_TARGET_DEPENDS ${json})

    if (NOT KCA_ADD_PLUGIN_INSTALL_NAMESPACE)
        message(FATAL_ERROR "Must specify INSTALL_NAMESPACE for ${plugin}")
    endif()
    install(TARGETS ${plugin} DESTINATION ${PLUGIN_INSTALL_DIR}/${KCA_ADD_PLUGIN_INSTALL_NAMESPACE})
endfunction()
