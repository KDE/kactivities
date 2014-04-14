# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (activitymanager-plugin-activityranking)

include_directories (
   ${CMAKE_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   )

set (
   activityranking_SRCS
   ActivityRanking.cpp
   ActivityRankingPlugin.cpp
   ActivityData.cpp
   Location.cpp
   ${plugin_implementation_SRCS}
   )

qt4_add_dbus_adaptor (
   activityranking_SRCS org.kde.ActivityManager.ActivityRanking.xml
   ActivityRanking.h ActivityRanking
   )

qt4_add_dbus_interface (
   activityranking_SRCS org.kde.LocationManager.xml
   LocationManagerInterface
   )

add_library (
   kactivitymanagerd_plugin_activityranking SHARED
   ${activityranking_SRCS}
   )

target_link_libraries (
   kactivitymanagerd_plugin_activityranking
   ${QT_QTSQL_LIBRARIES}
   )

install (TARGETS kactivitymanagerd_plugin_activityranking DESTINATION ${KAMD_PLUGIN_DIR})
# install (FILES kactivitymanagerd-plugin-activityranking.desktop DESTINATION ${SERVICES_INSTALL_DIR})

