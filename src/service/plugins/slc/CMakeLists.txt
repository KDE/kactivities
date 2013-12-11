# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (activitymanager-plugin-slc)

# kservice_desktop_to_json (activitymanager-plugin-slc.desktop)

include_directories (
   ${CMAKE_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KConfig_INCLUDE_DIR}
   )

set (
   slc_SRCS
   SlcPlugin.cpp
   ${plugin_implementation_SRCS}
   )

qt5_add_dbus_adaptor (
   slc_SRCS org.kde.ActivityManager.SLC.xml
   SlcPlugin.h SlcPlugin
   )

add_library (
   activitymanager_plugin_slc SHARED
   ${slc_SRCS}
   )

target_link_libraries (
   activitymanager_plugin_slc
   Qt5::Core
   Qt5::DBus
   KF5::ConfigCore
   KF5::DBusAddons
   )

set_target_properties(
   activitymanager_plugin_slc
   PROPERTIES PREFIX ""
   )

install (TARGETS activitymanager_plugin_slc DESTINATION ${KAMD_PLUGIN_DIR})
# install (FILES activitymanager-plugin-slc.desktop DESTINATION ${SERVICES_INSTALL_DIR})

