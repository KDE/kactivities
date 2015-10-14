# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivitymanagerd-plugin-activitytemplates)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KConfig_INCLUDE_DIR}
   )

set (
   activitytemplates_SRCS
   TemplatesPlugin.cpp
   ${plugin_implementation_SRCS}
   )

qt5_add_dbus_adaptor (
   activitytemplates_SRCS org.kde.ActivityManager.Templates.xml
   TemplatesPlugin.h TemplatesPlugin
   )

add_library (
   kactivitymanagerd_plugin_activitytemplates SHARED
   ${activitytemplates_SRCS}
   )

target_link_libraries (
   kactivitymanagerd_plugin_activitytemplates
   Qt5::Core
   Qt5::DBus
   KF5::ConfigCore
   KF5::DBusAddons
   KF5::CoreAddons
   )

set_target_properties (
   kactivitymanagerd_plugin_activitytemplates
   PROPERTIES PREFIX ""
   )

kcoreaddons_desktop_to_json (
   kactivitymanagerd_plugin_activitytemplates
   kactivitymanagerd-plugin-activitytemplates.desktop
   COMPAT_MODE DEFAULT_SERVICE_TYPE
   )

install (
   TARGETS kactivitymanagerd_plugin_activitytemplates
   DESTINATION ${KAMD_PLUGIN_DIR}
   )
install (
   FILES kactivitymanagerd-plugin-activitytemplates.desktop
   DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
   )

