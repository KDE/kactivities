project (activitymanager-globalshortcuts)

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    )

set (
    globalshortcuts_SRCS
    GlobalShortcutsPlugin.cpp
    ${plugin_implementation_SRCS}
    )

kde4_add_plugin (
    activitymanager_plugin_globalshortcuts
    ${globalshortcuts_SRCS}
    )

target_link_libraries (
    activitymanager_plugin_globalshortcuts
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KACTIVITIES_LIBS}
    )

install (TARGETS activitymanager_plugin_globalshortcuts DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES activitymanager-plugin-globalshortcuts.desktop DESTINATION ${SERVICES_INSTALL_DIR})

