# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesStatsTest)

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Test Core DBus Sql)

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
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/lib/core/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/lib/stats/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
   ${CMAKE_BINARY_DIR}/src/lib/core/
   ${CMAKE_BINARY_DIR}/src/lib/stats/
   ${SQLITE_INCLUDE_DIR}
   )

set (
   KActivitiesStatsTest_SRCS
   main.cpp
   QueryTest.cpp
   ResultSetTest.cpp
   ResultSetQuickCheckTest.cpp
   ResultWatcherTest.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/utils/qsqlquery_iterator.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/database/Database.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/database/schema/ResourcesDatabaseSchema.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/common/test.cpp
   )

if (NOT WIN32)

   add_executable (
      KActivitiesStatsTest
      ${KActivitiesStatsTest_SRCS}
      )

   target_link_libraries (
      KActivitiesStatsTest
      Qt5::Core
      Qt5::Test
      Qt5::DBus
      Qt5::Sql
      KF5::Activities
      KF5::ActivitiesExperimentalStats
      ${SQLITE_LIBRARIES}
      )

endif ()
