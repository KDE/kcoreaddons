# always install the mime-types
install(FILES kde5.xml DESTINATION ${KDE_INSTALL_MIMEDIR})

find_package(SharedMimeInfo 1.10)
set_package_properties(SharedMimeInfo PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Allows KDE applications to determine file types"
                      )

# update XDG mime-types if shared mime info is around
if(SharedMimeInfo_FOUND)
    update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})
endif()
