# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

cmake_minimum_required (VERSION 2.8.11)

project (KACTIVITIES)

set(REQUIRED_QT_VERSION 5.2.0)

# We don't build in-source
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message (
      FATAL_ERROR
      "kactivities require an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there."
   )
endif ()

# Extra CMake stuff
find_package (ECM 0.0.12 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(GenerateExportHeader)
include(ECMGenerateHeaders)

# Qt
set (CMAKE_AUTOMOC ON)
find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core DBus)

# KDE Frameworks
set(KF5_VERSION "4.98.0") # handled by release scripts # handled by release scripts
set(KF5_DEP_VERSION "4.98.0") # handled by release scripts
find_package (KF5DBusAddons ${KF5_DEP_VERSION} CONFIG REQUIRED)

# Basic includes
include (CPack)
include (FeatureSummary)
include (ECMPackageConfigHelpers)
include (ECMSetupVersion)


# Adding local CMake modules
set (
   CMAKE_MODULE_PATH
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
   ${CMAKE_MODULE_PATH}
)

ecm_setup_version(${KF5_VERSION}
                  VARIABLE_PREFIX KACTIVITIES
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kactivities_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesConfigVersion.cmake"
                  SOVERSION 5)

add_definitions(-DTRANSLATION_DOMAIN=\"kactivities5\")
add_subdirectory (src)
add_subdirectory (autotests)
add_subdirectory (tests)

set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Activities")

install(EXPORT KF5ActivitiesLibraryTargets
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   FILE KF5ActivitiesLibraryTargets.cmake
   NAMESPACE KF5::
   )

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/KF5Activities")

ecm_configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF5ActivitiesConfig.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              PATH_VARS  KF5_INCLUDE_INSTALL_DIR CMAKE_INSTALL_PREFIX )

                           install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesConfig.cmake"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kactivities_version.h
        DESTINATION ${KF5_INCLUDE_INSTALL_DIR} COMPONENT Devel )

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

