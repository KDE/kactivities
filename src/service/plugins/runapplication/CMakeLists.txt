# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivitymanagerd-runapplication)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KConfig_INCLUDE_DIR}
   )

set (
   runapplication_SRCS
   RunApplicationPlugin.cpp
   ${plugin_implementation_SRCS}
   )

add_library (
   kactivitymanagerd_plugin_runapplication SHARED
   ${runapplication_SRCS}
   )

target_link_libraries (
   kactivitymanagerd_plugin_runapplication
   Qt5::Core
   Qt5::DBus
   Qt5::Gui
   KF5::CoreAddons
   KF5::ConfigCore
   KF5::Service
   )

kservice_desktop_to_json (
   kactivitymanagerd_plugin_runapplication
   kactivitymanagerd-plugin-runapplication.desktop
   )


set_target_properties (
   kactivitymanagerd_plugin_runapplication
   PROPERTIES PREFIX ""
   )

install (
   TARGETS kactivitymanagerd_plugin_runapplication
   DESTINATION ${KAMD_PLUGIN_DIR}
   )
install (
   FILES kactivitymanagerd-plugin-runapplication.desktop
   DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
   )

