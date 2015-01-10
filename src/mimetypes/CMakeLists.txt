# for KDE frameworks 5 we require at least version 0.70
find_package(SharedMimeInfo 0.70)
set_package_properties(SharedMimeInfo PROPERTIES
                       TYPE REQUIRED
                       PURPOSE "Allows KDE applications to determine file types"
                      )

########### install files ###############
install(FILES kde5.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})
