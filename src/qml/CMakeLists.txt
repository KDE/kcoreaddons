ecm_add_qml_module(kcoreaddonsplugin URI org.kde.coreaddons CLASS_NAME KCoreAddonsPlugin INSTALLED_PLUGIN_TARGET KF6::kcoreaddonsplugin)
target_sources(kcoreaddonsplugin PRIVATE
    formats.cpp
    formats.h
    kcoreaddonsplugin.cpp
    kuserproxy.cpp
    kuserproxy.h
)
target_link_libraries(kcoreaddonsplugin PRIVATE
    Qt6::Core
    Qt6::Qml
    Qt6::Network
    KF6::CoreAddons
)

ecm_finalize_qml_module(kcoreaddonsplugin DESTINATION ${KDE_INSTALL_QMLDIR} EXPORT KF6CoreAddonsTargets)
