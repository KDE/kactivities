# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivitymanagerd-plugin-sqlite)

set (
   sqliteplugin_SRCS
   Database.cpp
   StatsPlugin.cpp
   ResourceScoreCache.cpp
   ResourceScoreMaintainer.cpp
   ResourceLinking.cpp
   # Rankings.cpp
   ${CMAKE_SOURCE_DIR}/src/utils/qsqlquery.cpp
   ${plugin_implementation_SRCS}
   )

qt5_add_dbus_adaptor (
   sqliteplugin_SRCS
   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   ResourceLinking.h ResourceLinking
   resourcelinkingadaptor
   )

include_directories (
   ${CMAKE_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}/../..
   ${KConfig_INCLUDE_DIR}
   )

add_library (
   kactivitymanagerd_plugin_sqlite SHARED
   ${sqliteplugin_SRCS}
   )

target_link_libraries (
   kactivitymanagerd_plugin_sqlite
   Qt5::Core
   Qt5::Sql
   KF5::ConfigCore
   KF5::DBusAddons
   KF5::CoreAddons
   )

set_target_properties(
   kactivitymanagerd_plugin_sqlite
   PROPERTIES PREFIX ""
   )

install (TARGETS kactivitymanagerd_plugin_sqlite DESTINATION ${KAMD_PLUGIN_DIR})
# install (FILES kactivitymanagerd-plugin-sqlite.desktop DESTINATION ${SERVICES_INSTALL_DIR})

