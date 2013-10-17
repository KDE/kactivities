# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivities)

# =======================================================
# Information to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0
# 0.1.1 => 0.1.1
# 0.2.0 => 0.2.0
# 6.1.0 => 6.1.0
# 6.2.0 => 6.2.0

# Library API version
set (KACTIVITIES_LIB_MAJOR_VERSION "6")
set (KACTIVITIES_LIB_MINOR_VERSION "2")
set (KACTIVITIES_LIB_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final relase. Can be used in production.
set (KACTIVITIES_LIB_SUFFIX_VERSION "")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
set (KACTIVITIES_LIB_SO_CUR_VERSION "6")
set (KACTIVITIES_LIB_SO_REV_VERSION "2")
set (KACTIVITIES_LIB_SO_AGE_VERSION "0")

# =======================================================
# Set env. variables accordinly.

set (KACTIVITIES_INCLUDE_DIR
   "${CMAKE_CURRENT_SOURCE_DIR}/.."
   "${CMAKE_CURRENT_SOURCE_DIR}/"
   CACHE STRING
   "Location of libkactivities headers" FORCE)

set (KACTIVITIES_LIBS
   "kactivities"
   CACHE STRING
   "Location of libkactivities binary" FORCE)

set (KACTIVITIES_LIB_VERSION_STRING "${KACTIVITIES_LIB_MAJOR_VERSION}.${KACTIVITIES_LIB_MINOR_VERSION}.${KACTIVITIES_LIB_PATCH_VERSION}${KACTIVITIES_LIB_SUFFIX_VERSION}")
set (KACTIVITIES_LIB_VERSION_ID "0x0${KACTIVITIES_LIB_MAJOR_VERSION}0${KACTIVITIES_LIB_MINOR_VERSION}0${KACTIVITIES_LIB_PATCH_VERSION}")
set (KACTIVITIES_LIB_SO_VERSION_STRING "${KACTIVITIES_LIB_SO_CUR_VERSION}.${KACTIVITIES_LIB_SO_REV_VERSION}.${KACTIVITIES_LIB_SO_AGE_VERSION}")

# =======================================================
# Now that we finished with the boilerplate, start
# with the library definition

set (
   kactivities_LIB_SRCS
   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.cpp
   consumer.cpp
   controller.cpp
   info.cpp
   manager_p.cpp
   resourceinstance.cpp
   version.cpp
   )

set_source_files_properties (
   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   PROPERTIES
   INCLUDE ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.h
   )

qt5_add_dbus_interface (
   kactivities_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   activities_interface
   )

qt5_add_dbus_interface (
   kactivities_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Resources.xml
   resources_interface
   )

qt5_add_dbus_interface (
   kactivities_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Features.xml
   features_interface
   )

qt5_add_dbus_interface (
   kactivities_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   resources_linking_interface
   )

add_library (
   KActivities SHARED
   ${kactivities_LIB_SRCS}
   )

include_directories (
   ${CMAKE_SOURCE_DIR}/src
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDBusAddons_INCLUDE_DIR}
   )

set_target_properties (
   KActivities
   PROPERTIES
   VERSION ${KACTIVITIES_LIB_SO_VERSION_STRING}
   SOVERSION ${KACTIVITIES_LIB_SO_CUR_VERSION}
   )

target_link_libraries (
   KActivities
   Qt5::Core
   Qt5::DBus
   KF5::KDBusAddons
   )

# install
generate_export_header(KActivities)

set (
   kactivities_LIB_HEADERS
   consumer.h
   controller.h
   info.h
   resourceinstance.h
   version.h
   ${CMAKE_CURRENT_BINARY_DIR}/kactivities_export.h
   )

set (
   kactivities_LIB_PRETTY_HEADERS
   includes/KActivities/Consumer
   includes/KActivities/Controller
   includes/KActivities/Info
   includes/KActivities/ResourceInstance
   includes/KActivities/Version
   )

install (
   FILES ${kactivities_LIB_HEADERS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/kactivities
   COMPONENT Devel
   )

install (
   FILES ${kactivities_LIB_PRETTY_HEADERS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KActivities
   COMPONENT Devel
   )

install (
   TARGETS KActivities
   EXPORT kactivitiesLibraryTargets
   ${INSTALL_TARGETS_DEFAULT_ARGS}
   )

install (
   EXPORT kactivitiesLibraryTargets
   DESTINATION ${LIB_INSTALL_DIR}/cmake/KActivities
   FILE KActivitiesLibraryTargets.cmake
   NAMESPACE KF5::
   )

if (NOT WIN32)
   configure_file (${CMAKE_CURRENT_SOURCE_DIR}/libkactivities.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libkactivities.pc)
   install (FILES ${CMAKE_CURRENT_BINARY_DIR}/libkactivities.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
endif ()

ecm_setup_version(5.0.0 VARIABLE_PREFIX KACTIVITIES
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kactivities_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KActivitiesConfigVersion.cmake")

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KActivities")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KActivitiesConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KActivitiesConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KActivitiesConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KActivitiesConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kactivities_version.h
        DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel )

