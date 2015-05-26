# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesStatsTestApp)

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets Test Quick QuickWidgets)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/lib/core/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/lib/stats/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
   ${CMAKE_BINARY_DIR}/src/lib/core/
   ${CMAKE_BINARY_DIR}/src/lib/stats/
   )

set (
   KActivitiesStatsTestApp_SRCS
   window.cpp
   main.cpp
   modeltest.cpp
   )

qt5_wrap_ui(
   KActivitiesStatsTestApp_SRCS
   window.ui
   )

qt5_add_resources(
   KActivitiesStatsTestApp_SRCS
   main.qrc
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
      Qt5::Test
      Qt5::Quick
      Qt5::QuickWidgets
      KF5::Activities
      KF5::ActivitiesExperimentalStats
      )

endif ()
