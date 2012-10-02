project (activitymanager-virtualdesktopswitch)

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    )

set (
    virtualdesktopswitch_SRCS
    VirtualDesktopSwitchPlugin.cpp
    ${plugin_implementation_SRCS}
    )

kde4_add_plugin (
    activitymanager_plugin_virtualdesktopswitch
    ${virtualdesktopswitch_SRCS}
    )

target_link_libraries (
    activitymanager_plugin_virtualdesktopswitch
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KACTIVITIES_LIBS}
    )

install (TARGETS activitymanager_plugin_virtualdesktopswitch DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES activitymanager-plugin-virtualdesktopswitch.desktop DESTINATION ${SERVICES_INSTALL_DIR})

