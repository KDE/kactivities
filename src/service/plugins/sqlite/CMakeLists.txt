# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivitymanagerd-plugin-sqlite)

# SQLite
set (SQLITE_MIN_VERSION 3.6.23)
find_package(Sqlite ${SQLITE_MIN_VERSION} REQUIRED)
set_package_properties(Sqlite PROPERTIES
   URL "http://www.sqlite.org"
   DESCRIPTION "The Sqlite database library"
   TYPE REQUIRED
   PURPOSE "Required by the Sqlite backend"
   )

set (
   sqliteplugin_SRCS
   Database.cpp
   StatsPlugin.cpp
   ResourceScoreCache.cpp
   ResourceScoreMaintainer.cpp
   ResourceLinking.cpp

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/database/Database.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/database/schema/ResourcesDatabaseSchema.cpp

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/utils/qsqlquery_iterator.cpp

   ${plugin_implementation_SRCS}
   )

qt5_add_dbus_adaptor (
   sqliteplugin_SRCS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   ResourceLinking.h ResourceLinking
   resourcelinkingadaptor
   )

qt5_add_dbus_adaptor (
   sqliteplugin_SRCS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesScoring.xml
   StatsPlugin.h StatsPlugin
   resourcescoringadaptor
   )

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}/../..
   ${KConfig_INCLUDE_DIR}
   ${SQLITE_INCLUDE_DIR}
   )

add_library (
   kactivitymanagerd_plugin_sqlite MODULE
   ${sqliteplugin_SRCS}
   )

target_link_libraries (
   kactivitymanagerd_plugin_sqlite
   Qt5::Core
   Qt5::Sql
   KF5::ConfigCore
   KF5::KIOCore
   KF5::DBusAddons
   KF5::CoreAddons
   KF5::Activities
   ${SQLITE_LIBRARIES}
   )

kservice_desktop_to_json (
   kactivitymanagerd_plugin_sqlite
   kactivitymanagerd-plugin-sqlite.desktop
   )

set_target_properties (
   kactivitymanagerd_plugin_sqlite
   PROPERTIES PREFIX ""
   )

install (
   TARGETS kactivitymanagerd_plugin_sqlite
   DESTINATION ${KAMD_PLUGIN_DIR}
   )
install (
   FILES kactivitymanagerd-plugin-sqlite.desktop
   DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
   )

