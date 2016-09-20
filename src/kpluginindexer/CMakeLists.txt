include(ECMMarkNonGuiExecutable)

set(kpluginindexer_SRCS main.cpp kpluginindexer.cpp)

ecm_qt_declare_logging_category(kpluginindexer_SRCS
                                HEADER kpluginindexer_debug.h
                                IDENTIFIER KPI
                                CATEGORY_NAME org.kde.kcoreaddons.kpluginindexer)

add_executable(kpluginindexer ${kpluginindexer_SRCS})
if(NOT CMAKE_TOOLCHAIN_FILE)
    add_executable(KF5::kpluginindexer ALIAS kpluginindexer)
endif()

# Mark it as non-gui so we won't create an app bundle on Mac OS X
ecm_mark_nongui_executable(kpluginindexer)

target_link_libraries(kpluginindexer Qt5::Core)

install(TARGETS kpluginindexer EXPORT KF5CoreAddonsToolingTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
