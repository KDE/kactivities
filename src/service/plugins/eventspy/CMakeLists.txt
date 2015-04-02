# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (activitymanager-eventspy)

find_package (KF5KIO ${KF5_DEP_VERSION} CONFIG REQUIRED)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KConfig_INCLUDE_DIR}
   )

set (
   evenyspy_SRCS
   EventSpy.cpp
   ${plugin_implementation_SRCS}
   )

add_library (
   kactivitymanagerd_plugin_eventspy SHARED
   ${evenyspy_SRCS}
   )

target_link_libraries (
   kactivitymanagerd_plugin_eventspy
   Qt5::Core
   Qt5::DBus
   KF5::ConfigCore
   KF5::CoreAddons
   KF5::KIOCore
   )

kservice_desktop_to_json (
   kactivitymanagerd_plugin_eventspy
   kactivitymanagerd-plugin-eventspy.desktop
   )

set_target_properties (
   kactivitymanagerd_plugin_eventspy
   PROPERTIES PREFIX ""
   )

install (
   TARGETS kactivitymanagerd_plugin_eventspy
   DESTINATION ${KAMD_PLUGIN_DIR}
   )
install (
   FILES kactivitymanagerd-plugin-eventspy.desktop
   DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
   )

