# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

cmake_minimum_required(VERSION 3.5)

# KDE Frameworks
set(KF5_VERSION "5.70.0") # handled by release scripts
set(KF5_DEP_VERSION "5.70.0") # handled by release scripts
project (KActivities VERSION ${KF5_VERSION})

option (KACTIVITIES_LIBRARY_ONLY "If true, compiles only the KActivities library, without the QML imports." OFF)
option (KACTIVITIES_ENABLE_EXCEPTIONS "If you have Boost 1.53, you need to build KActivities with exceptions enabled. This is UNTESTED and EXPERIMENTAL!" OFF)

set (REQUIRED_QT_VERSION 5.12.0)

# We don't build in-source
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message (
      FATAL_ERROR
      "kactivities require an out of source build. Please create a separate build directory and run 'cmake path_to_sources [options]' there."
   )
endif ()

set (KACTIVITIES_CURRENT_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Extra CMake stuff
include(FeatureSummary)
find_package(ECM 5.70.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include (KDEInstallDirs)
include (KDECMakeSettings)
include (KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include (GenerateExportHeader)
include (ECMGenerateHeaders)
include (ECMQtDeclareLoggingCategory)
include (ECMAddQch)
include (ECMMarkNonGuiExecutable)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

# Qt
set (CMAKE_AUTOMOC ON)
find_package (Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED COMPONENTS Core DBus)

# Basic includes
include (CPack)

include (CMakePackageConfigHelpers)
include (ECMSetupVersion)

message ("We are using the ${CMAKE_CXX_COMPILER_ID} compiler")
if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (KACTIVITIES_OVERRIDE_VISIBILITY STREQUAL "default"))
   message ("Setting visibility preset to default")
   set(CMAKE_CXX_VISIBILITY_PRESET default)
   set(CMAKE_VISIBILITY_INLINES_HIDDEN 0)
   string (REPLACE "-fvisibility-inlines-hidden" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
   string (REPLACE "-fvisibility=hidden" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif ()

# libKActivities

ecm_setup_version (
   PROJECT
   VARIABLE_PREFIX KACTIVITIES
   VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kactivities_version.h"
   PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5ActivitiesConfigVersion.cmake"
   SOVERSION 5
   )

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050d00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054400)
add_definitions(-DQT_NO_FOREACH)

add_subdirectory (src)
if (BUILD_TESTING)
    add_subdirectory (autotests)
    add_subdirectory (tests)
endif()

set (CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Activities")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF5Activities_QCH
        FILE KF5ActivitiesLibraryQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF5ActivitiesLibraryQchTargets.cmake\")")
endif()

install (
   EXPORT KF5ActivitiesLibraryTargets
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   FILE KF5ActivitiesLibraryTargets.cmake
   NAMESPACE KF5::
   )

configure_package_config_file (
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

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

