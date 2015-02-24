# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesStatsTestApp)

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)

include_directories (
   ${CMAKE_SOURCE_DIR}/src/lib/core/
   ${CMAKE_BINARY_DIR}/src/lib/core/
   ${CMAKE_SOURCE_DIR}/src/lib/stats/
   ${CMAKE_BINARY_DIR}/src/lib/stats/
   ${CMAKE_SOURCE_DIR}/src/
   ${CMAKE_SOURCE_DIR}/autotests/
   )

set (
   KActivitiesStatsTestApp_SRCS
   main.cpp
   )

if (NOT WIN32)

   add_executable (
      KActivitiesStatsTestApp
      ${KActivitiesStatsTestApp_SRCS}
      )

   target_link_libraries (
      KActivitiesStatsTestApp
      Qt5::Core
      Qt5::Gui
      Qt5::Widgets
      KF5::Activities
      KF5::ActivitiesStats
      )

endif ()
