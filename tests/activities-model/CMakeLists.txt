# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesModelTestApp)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)
find_package (KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS WindowSystem)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
   )

set (
   KActivitiesModelTestApp_SRCS
   window.cpp
   main.cpp
   )

qt_wrap_ui(
   KActivitiesModelTestApp_SRCS
   window.ui
   )

if (NOT WIN32)

   add_executable (
      KActivitiesModelTestApp
      ${KActivitiesModelTestApp_SRCS}
      )

   target_link_libraries (
      KActivitiesModelTestApp
      Qt6::Core
      Qt6::Gui
      Qt6::Widgets
      Qt6::DBus
      KF6::Activities
      KF6::WindowSystem
      )

endif ()
