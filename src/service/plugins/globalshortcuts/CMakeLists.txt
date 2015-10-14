# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (activitymanager-globalshortcuts)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
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
   kactivitymanagerd_plugin_globalshortcuts SHARED
   ${globalshortcuts_SRCS}
   )

target_link_libraries (
   kactivitymanagerd_plugin_globalshortcuts
   Qt5::Core
   Qt5::DBus
   Qt5::Widgets
   KF5::ConfigCore
   KF5::GlobalAccel
   KF5::I18n
   KF5::XmlGui
   KF5::CoreAddons
   )

kcoreaddons_desktop_to_json (
   kactivitymanagerd_plugin_globalshortcuts
   kactivitymanagerd-plugin-globalshortcuts.desktop
   COMPAT_MODE DEFAULT_SERVICE_TYPE
   )

set_target_properties (
   kactivitymanagerd_plugin_globalshortcuts
   PROPERTIES PREFIX ""
   )

install (
   TARGETS kactivitymanagerd_plugin_globalshortcuts
   DESTINATION ${KAMD_PLUGIN_DIR}
   )
install (
   FILES kactivitymanagerd-plugin-globalshortcuts.desktop
   DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
   )

