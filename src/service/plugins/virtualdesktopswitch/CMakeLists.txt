# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (activitymanager-virtualdesktopswitch)

include_directories (
   ${CMAKE_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KConfig_INCLUDE_DIR}
   )

set (
   virtualdesktopswitch_SRCS
   VirtualDesktopSwitchPlugin.cpp
   ${plugin_implementation_SRCS}
   )

add_library (
   activitymanager_plugin_virtualdesktopswitch SHARED
   ${virtualdesktopswitch_SRCS}
   )

target_link_libraries (
   activitymanager_plugin_virtualdesktopswitch
   Qt5::Core
   )

set_target_properties(
   activitymanager_plugin_virtualdesktopswitch
   PROPERTIES PREFIX ""
   )

install (TARGETS activitymanager_plugin_virtualdesktopswitch DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES activitymanager-plugin-virtualdesktopswitch.desktop DESTINATION ${SERVICES_INSTALL_DIR})

