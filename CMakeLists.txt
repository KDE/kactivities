# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

cmake_minimum_required (VERSION 2.8.11)

project (KACTIVITIES)

# We don't build in-source
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message (
      FATAL_ERROR
      "kactivities require an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there."
   )
endif ()

# Extra CMake stuff
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})


# Qt
set (CMAKE_AUTOMOC ON)
find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core DBus)

# KDE Frameworks
find_package (KF5 REQUIRED MODULE
   COMPONENTS
   CMake Compiler InstallDirs KDBusAddons
   )

# Enable C++11
if (UNIX)
   add_definitions ("-Wall -std=c++0x")
endif ()


# Basic includes
include (CPack)
include (FeatureSummary)
include (CMakePackageConfigHelpers)
include (ECMSetupVersion)


# Adding local CMake modules
set (
   CMAKE_MODULE_PATH
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
   ${CMAKE_MODULE_PATH}
)

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

add_subdirectory (src)

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

