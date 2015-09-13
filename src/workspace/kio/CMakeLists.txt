# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kioslave-activities)

# SQLite
set (SQLITE_MIN_VERSION 3.6.23)
find_package(Sqlite ${SQLITE_MIN_VERSION} REQUIRED)
set_package_properties(Sqlite PROPERTIES
   URL "http://www.sqlite.org"
   DESCRIPTION "The Sqlite database library"
   TYPE REQUIRED
   PURPOSE "Required by the Sqlite backend"
   )

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${SQLITE_INCLUDE_DIR}
   )

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Sql)
find_package (KF5 REQUIRED COMPONENTS KIO I18n)

########### next target ###############

set (kio_activities_PART_SRCS
   KioActivities.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/utils/qsqlquery_iterator.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/database/Database.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/database/schema/ResourcesDatabaseSchema.cpp
   )

add_library (
   kio_activities MODULE
   ${kio_activities_PART_SRCS}
   )

target_link_libraries (
   kio_activities
   Qt5::Sql
   KF5::KIOCore
   KF5::DBusAddons
   KF5::Activities
   KF5::I18n
   ${SQLITE_LIBRARIES}
   )

install (
   TARGETS kio_activities
   DESTINATION ${KDE_INSTALL_PLUGINDIR}
   )

########### install files ###############

install (
   FILES activities.protocol
   DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
   )
