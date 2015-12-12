# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

cmake_minimum_required (VERSION 2.8.11)

project (KActivities)

option (KACTIVITIES_LIBRARY_ONLY "If true, compiles only the KActivities library, without the service and other modules." OFF)
option (KACTIVITIES_ENABLE_EXCEPTIONS "If you have Boost 1.53, you need to build KActivities with exceptions enabled. This is UNTESTED and EXPERIMENTAL!" OFF)

option (KACTIVITIES_INSTALL_EXPERIMENTAL_LIBRARIES "If true, it will build and install the experimental libraries." OFF)
option (KACTIVITIES_INSTALL_EXPERIMENTAL_HEADERS "If true, it will install the headers of experimental libraries." OFF)

if (NOT KACTIVITIES_INSTALL_EXPERIMENTAL_LIBRARIES)
   set (KACTIVITIES_INSTALL_EXPERIMENTAL_HEADERS FALSE)
else()
   message (WARNING "**********************************************************\n"
                    "WARNING: The experimental libraries should not be used in"
                    "stable software releases. API and ABI can be changed"
                    "without notice. Do *NOT* make packages with this enabled!\n"
                    "**********************************************************")
endif()

set (REQUIRED_QT_VERSION 5.3.0)

# We don't build in-source
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message (
      FATAL_ERROR
      "kactivities require an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there."
   )
endif ()

set (KACTIVITIES_CURRENT_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Extra CMake stuff
include(FeatureSummary)
find_package(ECM 5.17.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://projects.kde.org/projects/kdesupport/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include (KDEInstallDirs)
include (KDECMakeSettings)
include (KDECompilerSettings)
include (GenerateExportHeader)
include (ECMGenerateHeaders)

# Qt
set (CMAKE_AUTOMOC ON)
find_package (Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED COMPONENTS Core DBus)

# KDE Frameworks
set(KF5_VERSION "5.18.0") # handled by release scripts
set(KF5_DEP_VERSION "5.17.0") # handled by release scripts
find_package (KF5DBusAddons ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5I18n ${KF5_DEP_VERSION} CONFIG REQUIRED)

# Basic includes
include (CPack)

include (ECMPackageConfigHelpers)
include (ECMSetupVersion)


# Adding local CMake modules
set (
   CMAKE_MODULE_PATH
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
   ${CMAKE_MODULE_PATH}
   )

add_definitions (-DTRANSLATION_DOMAIN=\"kactivities5\")
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install (po)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   set(CMAKE_CXX_VISIBILITY_PRESET default)
   set(CMAKE_VISIBILITY_INLINES_HIDDEN 0)
endif ()

# libKActivities

ecm_setup_version (
   ${KF5_VERSION}
   VARIABLE_PREFIX KACTIVITIES
   VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kactivities_version.h"
   PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesConfigVersion.cmake"
   SOVERSION 5
   )

set (CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Activities")

install (
   EXPORT KF5ActivitiesLibraryTargets
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   FILE KF5ActivitiesLibraryTargets.cmake
   NAMESPACE KF5::
   )

ecm_configure_package_config_file (
   "${CMAKE_CURRENT_SOURCE_DIR}/KF5ActivitiesConfig.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesConfig.cmake"
   INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
   PATH_VARS  KF5_INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
   )

install (
   FILES "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesConfig.cmake"
         "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesConfigVersion.cmake"
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   COMPONENT Devel
   )

install (
   FILES ${CMAKE_CURRENT_BINARY_DIR}/kactivities_version.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
   )

# libKActivitiesStats

if (KACTIVITIES_INSTALL_EXPERIMENTAL_HEADERS)
   ecm_setup_version (
      ${KF5_VERSION}
      VARIABLE_PREFIX KACTIVITIESEXPERIMENTALSTATS
      VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kactivitiesexperimentalstats_version.h"
      PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesExperimentalStatsConfigVersion.cmake"
      SOVERSION 1
      )

   set (CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5ActivitiesExperimentalStats")

   install (
      EXPORT KF5ActivitiesExperimentalStatsLibraryTargets
      DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
      FILE KF5ActivitiesExperimentalStatsLibraryTargets.cmake
      NAMESPACE KF5::
      )

   ecm_configure_package_config_file (
      "${CMAKE_CURRENT_SOURCE_DIR}/KF5ActivitiesExperimentalStatsConfig.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesExperimentalStatsConfig.cmake"
      INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
      PATH_VARS  KF5_INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX
      )

   install (
      FILES "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesExperimentalStatsConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesExperimentalStatsConfigVersion.cmake"
      DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
      COMPONENT Devel
      )

   install (
      FILES ${CMAKE_CURRENT_BINARY_DIR}/kactivitiesexperimentalstats_version.h
      DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5} COMPONENT Devel
      )
endif()

add_subdirectory (src)
add_subdirectory (autotests)
add_subdirectory (tests)

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

