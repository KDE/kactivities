# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (activitymanager-plugin-activitytemplates)

include_directories (
   ${CMAKE_SOURCE_DIR}/src
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

add_library (
   activitymanager_plugin_activitytemplates SHARED
   ${activitytemplates_SRCS}
   )

target_link_libraries (
   activitymanager_plugin_activitytemplates
   Qt5::Core
   Qt5::DBus
   KF5::KConfigCore
   KF5::KDBusAddons
   )

set_target_properties(
   activitymanager_plugin_activitytemplates
   PROPERTIES PREFIX ""
   )

install (TARGETS activitymanager_plugin_activitytemplates DESTINATION ${KAMD_PLUGIN_DIR})
# install (FILES activitymanager-plugin-activitytemplates.desktop DESTINATION ${SERVICES_INSTALL_DIR})

