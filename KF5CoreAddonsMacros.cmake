#
# kcoreaddons_desktop_to_json(target desktopfile
#                             DEFAULT_SERVICE_TYPE | SERVICE_TYPES <file> [<file> [...]]
#                             [OUTPUT_DIR dir] [COMPAT_MODE])
#
# This macro uses desktoptojson to generate a json file from a plugin
# description in a .desktop file. The generated file can be compiled
# into the plugin using the K_PLUGIN_FACTORY_WITH_JSON (cpp) macro.
#
# All files in SERVICE_TYPES will be parsed by desktoptojson to ensure that the generated
# json uses the right data type (string, string list, int, double or bool) for all of the
# properties. If your application does not have any custom properties defined you should pass
# DEFAULT_SERVICE_TYPE instead. It is an error if neither of these arguments is given.
# This is done in order to ensure that all applications explicitly choose the right service
# type and don't have runtime errors because of the data being wrong (QJsonValue does not
# perform any type conversions).
#
# If COMPAT_MODE is passed as an argument the generated JSON file will be compatible with
# the metadata format used by KPluginInfo (from KService), otherwise it will default to
# the new format that is used by KPluginMetaData (from KCoreAddons).
#
# If OUTPUT_DIR is set the generated file will be created inside <dir> instead of in
# ${CMAKE_CURRENT_BINARY_DIR}
#
# Example:
#
#  kcoreaddons_desktop_to_json(plasma_engine_time plasma-dataengine-time.desktop
#                              SERVICE_TYPES plasma-dataengine.desktop)

function(kcoreaddons_desktop_to_json target desktop)
    get_filename_component(desktop_basename ${desktop} NAME_WE) # allow passing an absolute path to the .desktop
    cmake_parse_arguments(DESKTOP_TO_JSON "COMPAT_MODE;DEFAULT_SERVICE_TYPE" "OUTPUT_DIR" "SERVICE_TYPES" ${ARGN})

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
    set(command KF5::desktoptojson -i ${desktop} -o ${json})
    if(DESKTOP_TO_JSON_COMPAT_MODE)
      list(APPEND command -c)
    endif()
    if(DESKTOP_TO_JSON_SERVICE_TYPES)
      foreach(type ${DESKTOP_TO_JSON_SERVICE_TYPES})
        if (EXISTS ${KDE_INSTALL_FULL_KSERVICETYPES5DIR}/${type})
            set(type ${KDE_INSTALL_FULL_KSERVICETYPES5DIR}/${type})
        endif()
        list(APPEND command -s ${type})
      endforeach()
    endif()

    file(RELATIVE_PATH relativejson ${CMAKE_CURRENT_BINARY_DIR} ${json})
    add_custom_command(
        OUTPUT ${json}
        COMMAND ${command}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${desktop}
        COMMENT "Generating ${relativejson}"
    )
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
# kcoreaddons_add_plugin(plugin_name
#     [SOURCES <src> [<src> [...]]] # optional since 5.83, required before
#     [JSON "pluginname.json"]
#     [INSTALL_NAMESPACE "servicename"]
# )
#
# This macro helps simplifying the creation of plugins for KPluginFactory
# based systems.
# It will create a plugin given the SOURCES list and the INSTALL_NAMESPACE so that
# the plugin is installed with the rest of the plugins from the same sub-system,
# within ${KDE_INSTALL_PLUGINDIR}.
# The JSON parameter is deprecated since 5.85, because it is not needed when the project is properly set up using
# the ECMSetupQtPluginMacroNames module. In case of plugin export macros provided by the KDE Frameworks this is already done and the parameter
# can be dropped with any older KF5 requirement.
# In case you generate the JSON files during the build it should be manually added to the AUTOGEN_TARGET_DEPENDS property,
# the kcoreaddons_desktop_to_json already does this for the generated file.
#
# Example:
#   kcoreaddons_add_plugin(kdeconnect_share SOURCES ${kdeconnect_share_SRCS} INSTALL_NAMESPACE "kdeconnect")
#
# Since 5.10.0

function(kcoreaddons_add_plugin plugin)
    set(options STATIC)
    set(oneValueArgs JSON INSTALL_NAMESPACE)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(KCA_ADD_PLUGIN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT KCA_ADD_PLUGIN_INSTALL_NAMESPACE)
        message(FATAL_ERROR "Must specify INSTALL_NAMESPACE for ${plugin}")
    endif()

    if (KCA_ADD_PLUGIN_STATIC)
        add_library(${plugin} OBJECT ${KCA_ADD_PLUGIN_SOURCES})
        target_compile_definitions(${plugin} PRIVATE QT_STATICPLUGIN)
        set(IMPORT_LINE "K_IMPORT_PLUGIN(QStringLiteral(\"${KCA_ADD_PLUGIN_INSTALL_NAMESPACE}\"), ${plugin}_factory)")
        set(KCOREADDONS_STATIC_PLUGINS ${KCOREADDONS_STATIC_PLUGINS} ${IMPORT_LINE} CACHE INTERNAL "list of known static plugins, used to generate K_IMPORT_PLUGIN macros")
    else()
        add_library(${plugin} MODULE ${KCA_ADD_PLUGIN_SOURCES})
    endif()

    if ("${ECM_GLOBAL_FIND_VERSION}" VERSION_GREATER_EQUAL "5.85.0" AND KCA_ADD_PLUGIN_JSON)
        message(WARNING "Setting the JSON parameter is deprecated, see function docs for details")
    endif()
    get_filename_component(json "${KCA_ADD_PLUGIN_JSON}" REALPATH)
    set_property(TARGET ${plugin} APPEND PROPERTY AUTOGEN_TARGET_DEPENDS ${json})


    if ("${ECM_GLOBAL_FIND_VERSION}" VERSION_GREATER_EQUAL "5.88.0")
        target_compile_definitions(${plugin} PRIVATE KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME=${plugin}_factory)
    endif()

    # If we have static plugins there are no plugins to install
    if (KCA_ADD_PLUGIN_STATIC)
        return()
    endif()
    # If find_package(ECM 5.38) or higher is called, output the plugin in a INSTALL_NAMESPACE subfolder.
    # See https://community.kde.org/Guidelines_and_HOWTOs/Making_apps_run_uninstalled
    if(NOT ("${ECM_GLOBAL_FIND_VERSION}" VERSION_LESS "5.88.0"))
        set_target_properties(${plugin} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/plugins/${KCA_ADD_PLUGIN_INSTALL_NAMESPACE}")
    elseif(NOT ("${ECM_GLOBAL_FIND_VERSION}" VERSION_LESS "5.38.0"))
        set_target_properties(${plugin} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${KCA_ADD_PLUGIN_INSTALL_NAMESPACE}")
    endif()

    if(NOT ANDROID)
        install(TARGETS ${plugin} DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KCA_ADD_PLUGIN_INSTALL_NAMESPACE})
    else()
        string(REPLACE "/" "_" pluginprefix "${KCA_ADD_PLUGIN_INSTALL_NAMESPACE}")
        set_property(TARGET ${plugin} PROPERTY PREFIX "libplugins_")
        if(NOT pluginprefix STREQUAL "")
            set_property(TARGET ${plugin} APPEND_STRING PROPERTY PREFIX "${pluginprefix}_")
        endif()
        install(TARGETS ${plugin} DESTINATION ${KDE_INSTALL_PLUGINDIR})
    endif()
endfunction()

function(kcoreaddons_target_static_plugins app_target)
    set(IMPORT_PLUGIN_STATEMENTS "#include <KPluginLoader>\n\n")
    foreach(PLUGIN_CLASS_NAME IN LISTS KCOREADDONS_STATIC_PLUGINS)
        set(IMPORT_PLUGIN_STATEMENTS ${IMPORT_PLUGIN_STATEMENTS} ${PLUGIN_CLASS_NAME}\n)
    endforeach()
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_static_plugins_tmp.cpp" ${IMPORT_PLUGIN_STATEMENTS})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_static_plugins_tmp.cpp" "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_static_plugins.cpp")
    file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_static_plugins_tmp.cpp")
    target_sources(${app_target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_static_plugins.cpp")
endfunction()
