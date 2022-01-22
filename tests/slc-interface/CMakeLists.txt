# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesSLCTestApp)

find_package (Qt${QT_MAJOR_VERSION} REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
   )

set (
   KActivitiesSLCTestApp_SRCS
   window.cpp
   main.cpp
   )

qt_add_dbus_interface (
   KActivitiesSLCTestApp_SRCS

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/service/plugins/slc/org.kde.ActivityManager.SLC.xml
   slc_interface
   )

qt_wrap_ui(
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
      Qt${QT_MAJOR_VERSION}::Core
      Qt${QT_MAJOR_VERSION}::Gui
      Qt${QT_MAJOR_VERSION}::Widgets
      Qt${QT_MAJOR_VERSION}::DBus
      KF5::Activities
      )

endif ()
