# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
cmake_minimum_required(VERSION 2.8.12)

project (KActivities)

# =======================================================
# Information to update before to release this library.

set(KF5_VERSION "4.96.0")

# =======================================================
# Now that we finished with the boilerplate, start
# with the library definition

set (
   KActivities_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.cpp

   consumer.cpp
   controller.cpp
   info.cpp
   resourceinstance.cpp

   mainthreadexecutor_p.cpp
   manager_p.cpp
   activitiescache_p.cpp
   dbusfuture_p.cpp
   debug_p.cpp

   version.cpp
   )

set_source_files_properties (
   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   PROPERTIES
   INCLUDE ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.h
   )

qt5_add_dbus_interface (
   KActivities_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   activities_interface
   )

qt5_add_dbus_interface (
   KActivities_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Resources.xml
   resources_interface
   )

qt5_add_dbus_interface (
   KActivities_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Features.xml
   features_interface
   )

qt5_add_dbus_interface (
   KActivities_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   resources_linking_interface
   )

add_library (
   KF5Activities SHARED
   ${KActivities_LIB_SRCS}
   )
add_library(KF5::Activities ALIAS KF5Activities)

include_directories (
   ${CMAKE_SOURCE_DIR}/src
   ${KDBusAddons_INCLUDE_DIR}
   )

set_target_properties (
   KF5Activities
   PROPERTIES
   VERSION ${KACTIVITIES_VERSION_STRING}
   SOVERSION ${KACTIVITIES_SOVERSION}
   EXPORT_NAME Activities
   )

target_link_libraries (
   KF5Activities
   PUBLIC
      Qt5::Core
   PRIVATE
      Qt5::DBus
      KF5::DBusAddons
   )

target_include_directories(KF5Activities INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/KActivities>" )

# install
generate_export_header(KF5Activities BASE_NAME KActivities)

ecm_generate_headers(KActivities_CamelCase_HEADERS
  HEADER_NAMES
  Consumer
  Controller
  Info
  ResourceInstance
  Version
  PREFIX KActivities
  REQUIRED_HEADERS KActivities_HEADERS
)
install(FILES ${KActivities_CamelCase_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/KActivities/KActivities COMPONENT Devel)

install(FILES
   ${KActivities_HEADERS}
   ${CMAKE_CURRENT_BINARY_DIR}/kactivities_export.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/KActivities/kactivities
   COMPONENT Devel
   )

install(TARGETS KF5Activities
   EXPORT KF5ActivitiesLibraryTargets
   ${INSTALL_TARGETS_DEFAULT_ARGS}
   )


if (NOT WIN32)
   configure_file (${CMAKE_CURRENT_SOURCE_DIR}/libKActivities.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libKActivities.pc)
   install (FILES ${CMAKE_CURRENT_BINARY_DIR}/libKActivities.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
endif ()

