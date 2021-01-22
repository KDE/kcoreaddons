add_subdirectory(lib)
add_subdirectory(mimetypes)

add_subdirectory(desktoptojson)

ecm_qt_install_logging_categories(
    EXPORT KCOREADDONS
    FILE kcoreaddons.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
