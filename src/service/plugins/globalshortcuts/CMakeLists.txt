# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (activitymanager-globalshortcuts)

include_directories (
   ${CMAKE_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KConfig_INCLUDE_DIR}
   )

set (
   globalshortcuts_SRCS
   GlobalShortcutsPlugin.cpp
   ${plugin_implementation_SRCS}
   )

add_library (
   activitymanager_plugin_globalshortcuts SHARED
   ${globalshortcuts_SRCS}
   )

target_link_libraries (
   activitymanager_plugin_globalshortcuts
   Qt5::Core
   )

set_target_properties(
   activitymanager_plugin_globalshortcuts
   PROPERTIES PREFIX ""
   )

install (TARGETS activitymanager_plugin_globalshortcuts DESTINATION ${KAMD_PLUGIN_DIR})
# install (FILES activitymanager-plugin-globalshortcuts.desktop DESTINATION ${SERVICES_INSTALL_DIR})

