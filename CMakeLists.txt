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
find_package (ECM 0.0.9 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

# Qt
set (CMAKE_AUTOMOC ON)
find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core DBus)

# KDE Frameworks
find_package (KF5 CONFIG REQUIRED DBusAddons)

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

add_subdirectory (src)
add_subdirectory (tests)

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

