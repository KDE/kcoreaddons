#
# kcoreaddons_desktop_to_json(target desktopfile
#                             DEFAULT_SERVICE_TYPE | SERVICE_TYPES <file> [<file> [...]]
#                             [OUTPUT_DIR dir | OUTPUT_FILE file] [COMPAT_MODE])
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
# If OUTPUT_FILE is set the generated file will be <file> instead of the default
# ${CMAKE_CURRENT_BINARY_DIR}/$(basename desktopfile).json
# .. note::
# This is only considered porting aid and will be removed in KF6. Please convert the desktop files
# in-source to json using the desktoptojson executable
#
# Example:
#
#  kcoreaddons_desktop_to_json(plasma_engine_time plasma-dataengine-time.desktop
#                              SERVICE_TYPES plasma-dataengine.desktop)

function(kcoreaddons_desktop_to_json target desktop)
    message(WARNING "kcoreaddons_desktop_to_json is deprecated and will be removed in KF6. Convert the desktop files to JSON in source using the desktoptojson executable")
    get_filename_component(desktop_basename ${desktop} NAME_WE) # allow passing an absolute path to the .desktop
    cmake_parse_arguments(DESKTOP_TO_JSON "COMPAT_MODE;DEFAULT_SERVICE_TYPE" "OUTPUT_DIR;OUTPUT_FILE" "SERVICE_TYPES" ${ARGN})

    if(DESKTOP_TO_JSON_OUTPUT_FILE)
        set(json "${DESKTOP_TO_JSON_OUTPUT_FILE}")
    elseif(DESKTOP_TO_JSON_OUTPUT_DIR)
        set(json "${DESKTOP_TO_JSON_OUTPUT_DIR}/${desktop_basename}.json")
    else()
        set(json "${CMAKE_CURRENT_BINARY_DIR}/${desktop_basename}.json")
    endif()

    kcoreaddons_desktop_to_json_crosscompilation_args(_crosscompile_args)
    set(command KF6::desktoptojson ${_crosscompile_args} -i ${desktop} -o ${json})
    if(DESKTOP_TO_JSON_COMPAT_MODE)
      list(APPEND command -c)
    endif()
    if(DESKTOP_TO_JSON_SERVICE_TYPES)
      foreach(type ${DESKTOP_TO_JSON_SERVICE_TYPES})
        if(NOT IS_ABSOLUTE "${type}")
          if(CMAKE_CROSSCOMPILING)
            if (DEFINED KSERVICETYPE_PATH_${type})
              set(_guess ${KSERVICETYPE_PATH_${type}})
            else()
              set(_guess ${CMAKE_SYSROOT}/${KDE_INSTALL_FULL_KSERVICETYPESDIR}/${type})
            endif()
            if(EXISTS ${_guess})
              set(type ${CMAKE_SYSROOT}/${KDE_INSTALL_FULL_KSERVICETYPESDIR}/${type})
            else()
              message(WARNING "Could not find service type ${type}, tried ${_guess}. Set KSERVICETYPE_PATH_${type} to override this guess.")
            endif()
          elseif(EXISTS ${KDE_INSTALL_FULL_KSERVICETYPESDIR}/${type})
            set(type ${KDE_INSTALL_FULL_KSERVICETYPESDIR}/${type})
          endif()
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

# Internal function to get the additional arguments that should be passed to desktoptojson when cross-compiling
function(kcoreaddons_desktop_to_json_crosscompilation_args output_var)
  set(_extra_args)
  # When cross-compiling we can't use relative paths since that might find an incompatible file on the host
  # using QStandardPaths (or not find any at all e.g. when cross-compiling from macOS).
  if(CMAKE_CROSSCOMPILING)
    set(_extra_args --strict-path-mode --generic-data-path "${CMAKE_SYSROOT}/${KDE_INSTALL_FULL_DATAROOTDIR}")
  endif()
  set(${output_var} ${_extra_args} PARENT_SCOPE)
endfunction()

#
# kcoreaddons_add_plugin(plugin_name
#     [SOURCES <src> [<src> [...]]] # optional since 5.83, required before
#     [JSON "pluginname.json"]
#     [STATIC]
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
# Since 5.89 the macro supports static plugins by passing in the STATIC option.
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
    if (KCA_ADD_PLUGIN_UNPARSED_ARGUMENTS)
            message(FATAL_ERROR "kcoreaddons_add_plugin method call recieved unexpected arguments: ${KCA_ADD_PLUGIN_UNPARSED_ARGUMENTS}")
    endif()

    string(REPLACE "-" "_" SANITIZED_PLUGIN_NAME ${plugin})
    string(REPLACE "." "_" SANITIZED_PLUGIN_NAME ${SANITIZED_PLUGIN_NAME})
    if (KCA_ADD_PLUGIN_STATIC)
        add_library(${plugin} STATIC ${KCA_ADD_PLUGIN_SOURCES})
        target_compile_definitions(${plugin} PRIVATE QT_STATICPLUGIN)
        set_property(TARGET ${plugin} PROPERTY AUTOMOC_MOC_OPTIONS -MX-KDE-FileName=${KCA_ADD_PLUGIN_INSTALL_NAMESPACE}/${plugin})
        string(REPLACE "/" "_" SANITIZED_PLUGIN_NAMESPACE ${KCA_ADD_PLUGIN_INSTALL_NAMESPACE})

        if (NOT ${SANITIZED_PLUGIN_NAME} IN_LIST KCOREADDONS_STATIC_PLUGINS${SANITIZED_PLUGIN_NAMESPACE})
            set(KCOREADDONS_STATIC_PLUGINS${SANITIZED_PLUGIN_NAMESPACE} "${KCOREADDONS_STATIC_PLUGINS${SANITIZED_PLUGIN_NAMESPACE}};${SANITIZED_PLUGIN_NAME}" CACHE INTERNAL "list of known static plugins for ${KCA_ADD_PLUGIN_INSTALL_NAMESPACE} namespace, used to generate Q_IMPORT_PLUGIN macros")
        endif()
    else()
        add_library(${plugin} MODULE ${KCA_ADD_PLUGIN_SOURCES})
    endif()

    target_compile_definitions(${plugin} PRIVATE KPLUGINFACTORY_PLUGIN_CLASS_INTERNAL_NAME=${SANITIZED_PLUGIN_NAME}_factory)

    # If we have static plugins there are no plugins to install
    if (KCA_ADD_PLUGIN_STATIC)
        return()
    endif()

    set_target_properties(${plugin} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${KCA_ADD_PLUGIN_INSTALL_NAMESPACE}")

    if (NOT KCOREADDONS_INTERNAL_SKIP_PLUGIN_INSTALLATION)
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
    endif()
endfunction()

# This macro imports the plugins for the given namespace that were
# registered using the kcoreaddons_add_plugin function.
# This includes the K_IMPORT_PLUGIN statements and linking the plugins to the given target.
#
# In case the plugins are used in both the executable and multiple autotests it it recommended to
# bundle the static plugins in a shared lib for the autotests. In case of shared libs the plugin registrations
# will take effect when the test link against it.
#
# Since 5.89
function(kcoreaddons_target_static_plugins app_target plugin_namespace)
    cmake_parse_arguments(ARGS "" "LINK_OPTION" "" ${ARGN})
    set(IMPORT_PLUGIN_STATEMENTS "#include <kstaticpluginhelpers.h>\n\n")
    string(REPLACE "/" "_" SANITIZED_PLUGIN_NAMESPACE ${plugin_namespace})
    set(TMP_PLUGIN_FILE "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_static_${SANITIZED_PLUGIN_NAMESPACE}_plugins_tmp.cpp")
    set(PLUGIN_FILE "${CMAKE_CURRENT_BINARY_DIR}/kcoreaddons_static_${SANITIZED_PLUGIN_NAMESPACE}_plugins.cpp")

    foreach(PLUGIN_TARGET_NAME IN LISTS KCOREADDONS_STATIC_PLUGINS${SANITIZED_PLUGIN_NAMESPACE})
        if (PLUGIN_TARGET_NAME)
            set(IMPORT_PLUGIN_STATEMENTS "${IMPORT_PLUGIN_STATEMENTS}K_IMPORT_PLUGIN(QStringLiteral(\"${plugin_namespace}\"), ${PLUGIN_TARGET_NAME}_factory)\;\n")
            target_link_libraries(${app_target} ${ARGS_LINK_OPTION} ${PLUGIN_TARGET_NAME})
        endif()
    endforeach()
    file(WRITE ${TMP_PLUGIN_FILE} ${IMPORT_PLUGIN_STATEMENTS})
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TMP_PLUGIN_FILE} ${PLUGIN_FILE})
    file(REMOVE ${TMP_PLUGIN_FILE})
    target_sources(${app_target} PRIVATE ${PLUGIN_FILE})
endfunction()

# Clear previously set plugins, otherwise Q_IMPORT_PLUGIN statements
# will fail to compile if plugins got removed from the build
get_directory_property(_cache_vars CACHE_VARIABLES)
foreach(CACHED_VAR IN LISTS _cache_vars)
    if (CACHED_VAR MATCHES "^KCOREADDONS_STATIC_PLUGINS")
        set(${CACHED_VAR} "" CACHE INTERNAL "")
    endif()
endforeach()