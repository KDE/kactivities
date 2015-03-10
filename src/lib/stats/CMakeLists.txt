# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (KActivitiesStats)

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Sql)

set (
   KActivitiesStats_LIB_SRCS

   query.cpp
   terms.cpp
   resultset.cpp
   resultwatcher.cpp
   resultmodel.cpp
   activitiessync_p.cpp

   ${CMAKE_SOURCE_DIR}/src/common/database/Database.cpp
   ${CMAKE_SOURCE_DIR}/src/common/database/schema/ResourcesDatabaseSchema.cpp
   ${CMAKE_SOURCE_DIR}/src/utils/qsqlquery_iterator.cpp

   version.cpp
   )

qt5_add_dbus_interface (
   KActivitiesStats_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesScoring.xml
   resourcesscoring_interface
   )

qt5_add_dbus_interface (
   KActivitiesStats_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   resourceslinking_interface
   )



add_library (
   KF5ActivitiesStats SHARED
   ${KActivitiesStats_LIB_SRCS}
   )
add_library(KF5::ActivitiesStats ALIAS KF5ActivitiesStats)

include_directories (
   ${CMAKE_SOURCE_DIR}/src
   ${KDBusAddons_INCLUDE_DIR}
   )

# This is an experimental library, keeping the SOVERSION at 0
# to reflect that
set_target_properties (
   KF5ActivitiesStats
   PROPERTIES
   VERSION "0.0.1"
   SOVERSION "0"
   EXPORT_NAME ActivitiesStats
   )

target_link_libraries (
   KF5ActivitiesStats
   PUBLIC
      Qt5::Core
   PRIVATE
      Qt5::DBus
      Qt5::Sql
      KF5::Activities
      KF5::DBusAddons
   )

target_include_directories (
   KF5ActivitiesStats
   INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KActivitiesStats>"
   )

# install
generate_export_header (KF5ActivitiesStats BASE_NAME KActivitiesStats)

ecm_generate_headers (
   KActivitiesStats_CamelCase_HEADERS
   HEADER_NAMES
   Query
   Terms
   ResultSet
   ResultWatcher
   ResultModel
   Version
   PREFIX KActivitiesStats
   REQUIRED_HEADERS KActivitiesStats_HEADERS
)
install (
   FILES ${KActivitiesStats_CamelCase_HEADERS}
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KActivities/KActivities/Stats
   COMPONENT Devel
   )

install (
   FILES ${KActivitiesStats_HEADERS}
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KActivities/kactivitiesstats
   COMPONENT Devel
   )

install (
   TARGETS KF5ActivitiesStats
   EXPORT KF5ActivitiesStatsLibraryTargets
   ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
   )


if (NOT WIN32)
   configure_file (${CMAKE_CURRENT_SOURCE_DIR}/libKActivitiesStats.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libKActivitiesStats.pc)
   install (
      FILES ${CMAKE_CURRENT_BINARY_DIR}/libKActivitiesStats.pc
      DESTINATION ${KDE_INSTALL_LIBDIR}/pkgconfig
      )
endif ()

include (ECMGeneratePriFile)
ecm_generate_pri_file (
   BASE_NAME KActivitiesStats
   LIB_NAME KF5ActivitiesStats
   DEPS "KDBusAddons"
   FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KF5_INCLUDE_INSTALL_DIR}/KActivitiesStats
   )
install (
   FILES ${PRI_FILENAME}
   DESTINATION ${ECM_MKSPECS_INSTALL_DIR}
   )


