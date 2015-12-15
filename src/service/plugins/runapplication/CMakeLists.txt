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

kcoreaddons_add_plugin (
   kactivitymanagerd_plugin_runapplication
   JSON kactivitymanagerd-plugin-runapplication.json
   SOURCES ${runapplication_SRCS}
   INSTALL_NAMESPACE ${KAMD_PLUGIN_DIR}
   )

target_link_libraries (
   kactivitymanagerd_plugin_runapplication
   Qt5::Core
   Qt5::DBus
   Qt5::Gui
   KF5::CoreAddons
   KF5::ConfigCore
   KF5::Service
   kactivitymanagerd_plugin
   )

set_target_properties (
   kactivitymanagerd_plugin_runapplication
   PROPERTIES PREFIX ""
   )

