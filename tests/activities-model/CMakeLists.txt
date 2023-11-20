# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesModelTestApp)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)
find_package (KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS WindowSystem)

if (NOT WIN32)

add_executable(KActivitiesModelTestApp)

target_include_directories(KActivitiesModelTestApp PRIVATE
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
)

qt_wrap_ui(KActivitiesModelTestApp_ui_SRCS
   window.ui
)

target_sources(KActivitiesModelTestApp PRIVATE
   ${KActivitiesModelTestApp_ui_SRCS}
   window.cpp
   main.cpp
)

target_link_libraries(KActivitiesModelTestApp
   PRIVATE
      Qt6::Core
      Qt6::Gui
      Qt6::Widgets
      Qt6::DBus
      KActivities
      KF6::WindowSystem
)

endif ()
