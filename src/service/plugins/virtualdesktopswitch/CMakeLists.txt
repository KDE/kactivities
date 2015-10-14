# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivitymanagerd-virtualdesktopswitch)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
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
   kactivitymanagerd_plugin_virtualdesktopswitch SHARED
   ${virtualdesktopswitch_SRCS}
   )

target_link_libraries (
   kactivitymanagerd_plugin_virtualdesktopswitch
   Qt5::Core
   Qt5::DBus
   KF5::ConfigCore
   KF5::WindowSystem
   KF5::CoreAddons
   )

set_target_properties (
   kactivitymanagerd_plugin_virtualdesktopswitch
   PROPERTIES PREFIX ""
   )

install (
   TARGETS kactivitymanagerd_plugin_virtualdesktopswitch
   DESTINATION ${KAMD_PLUGIN_DIR}
   )
