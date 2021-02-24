add_subdirectory(lib)
add_subdirectory(mimetypes)

add_subdirectory(desktoptojson)

ecm_install_qloggingcategories_metadata(
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
