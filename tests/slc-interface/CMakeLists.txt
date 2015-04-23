# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesSLCTestApp)

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/lib/core/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
   ${CMAKE_BINARY_DIR}/src/lib/core/
   )

set (
   KActivitiesSLCTestApp_SRCS
   window.cpp
   main.cpp
   )

qt5_wrap_ui(
   KActivitiesSLCTestApp_SRCS
   window.ui
   )

if (NOT WIN32)

   add_executable (
      KActivitiesSLCTestApp
      ${KActivitiesSLCTestApp_SRCS}
      )

   target_link_libraries (
      KActivitiesSLCTestApp
      Qt5::Core
      Qt5::Gui
      Qt5::Widgets
      KF5::Activities
      )

endif ()
