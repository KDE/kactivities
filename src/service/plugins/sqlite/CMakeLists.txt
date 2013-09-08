# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (activitymanager-plugin-sqlite)

set (
   sqliteplugin_SRCS
   DatabaseConnection.cpp
   StatsPlugin.cpp
   ResourceScoreCache.cpp
   ResourceScoreMaintainer.cpp
   ${plugin_implementation_SRCS}
   )

# TODO: We don't really have users of this one
# If we get them, enable this again in non debug builds
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if (
      (CMAKE_BUILD_TYPE STREQUAL "Debug") OR
      (CMAKE_BUILD_TYPE STREQUAL "DebugFull")
   )
   set (sqliteplugin_SRCS
      ${sqliteplugin_SRCS}
      Rankings.cpp
      )
endif ()

qt5_add_dbus_adaptor (
   sqliteplugin_SRCS
   org.kde.ActivityManager.Resources.Scoring.xml
   StatsPlugin.h StatsPlugin
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
   activitymanager_plugin_sqlite SHARED
   ${sqliteplugin_SRCS}
   )

target_link_libraries (
   activitymanager_plugin_sqlite
   Qt5::Core
   Qt5::Sql
   KF5::KConfigCore
   KF5::KDBusAddons
   )

set_target_properties(
   activitymanager_plugin_sqlite
   PROPERTIES PREFIX ""
   )

install (TARGETS activitymanager_plugin_sqlite DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES activitymanager-plugin-sqlite.desktop DESTINATION ${SERVICES_INSTALL_DIR})

