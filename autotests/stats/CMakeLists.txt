# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesStatsTest)

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Test Core DBus Sql)

include_directories (
   ${CMAKE_SOURCE_DIR}/src/lib/core/
   ${CMAKE_BINARY_DIR}/src/lib/core/
   ${CMAKE_SOURCE_DIR}/src/lib/stats/
   ${CMAKE_BINARY_DIR}/src/lib/stats/
   ${CMAKE_SOURCE_DIR}/src/
   ${CMAKE_SOURCE_DIR}/tests/
   )

set (
   KActivitiesStatsTest_SRCS
   main.cpp
   QueryTest.cpp
   ResultSetTest.cpp
   ${CMAKE_SOURCE_DIR}/tests/common/test.cpp
   ${CMAKE_SOURCE_DIR}/src/common/database/Database.cpp
   ${CMAKE_SOURCE_DIR}/src/common/database/schema/ResourcesDatabaseSchema.cpp
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
      KF5::ActivitiesStats
      )

endif ()
