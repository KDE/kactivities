# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project(KActivitiesStats)
set(PROJECT_VERSION "5.27.80")

cmake_minimum_required(VERSION 3.16)

set(QT_MIN_VERSION "6.4.0")
set(KF6_MIN_VERSION "5.240.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} )

cmake_minimum_required(VERSION 3.16)

option (KACTIVITIES_LIBRARY_ONLY "If true, compiles only the KActivities library, without the QML imports." OFF)
option (KACTIVITIES_ENABLE_EXCEPTIONS "If you have Boost 1.53, you need to build KActivities with exceptions enabled. This is UNTESTED and EXPERIMENTAL!" OFF)

# We don't build in-source
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message (
      FATAL_ERROR
      "kactivities require an out of source build. Please create a separate build directory and run 'cmake path_to_sources [options]' there."
   )
endif ()

set (KACTIVITIES_CURRENT_ROOT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(FeatureSummary)
include (KDEInstallDirs)
include (KDECMakeSettings)
include(KDEGitCommitHooks)
include (KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include (ECMGenerateExportHeader)
include (ECMGenerateHeaders)
include (ECMGeneratePkgConfigFile)
include (ECMQtDeclareLoggingCategory)
include (ECMAddQch)
include (ECMMarkNonGuiExecutable)
include(ECMDeprecationSettings)
include (ECMQmlModule)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

# Qt
set (CMAKE_AUTOMOC ON)
find_package (Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core DBus)

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

set(kactivities_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/lib/kactivities_version.h")
ecm_setup_version (
   PROJECT
   VARIABLE_PREFIX KACTIVITIES
   VERSION_HEADER "${kactivities_version_header}"
   PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6ActivitiesConfigVersion.cmake"
   SOVERSION 6
   )

ecm_set_disabled_deprecation_versions(
    QT 6.4
    KF 5.102.0
)

add_subdirectory (src)
if (BUILD_TESTING)
    add_subdirectory (autotests)
    add_subdirectory (tests)
endif()

set (CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Activities")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Activities_QCH
        FILE KF6ActivitiesLibraryQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6ActivitiesLibraryQchTargets.cmake\")")
endif()

install (
   EXPORT KF6ActivitiesLibraryTargets
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   FILE KF6ActivitiesLibraryTargets.cmake
   NAMESPACE KF6::
   )

configure_package_config_file (
   "${CMAKE_CURRENT_SOURCE_DIR}/KF6ActivitiesConfig.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/KF6ActivitiesConfig.cmake"
   INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
   )

install (
   FILES "${CMAKE_CURRENT_BINARY_DIR}/KF6ActivitiesConfig.cmake"
         "${CMAKE_CURRENT_BINARY_DIR}/KF6ActivitiesConfigVersion.cmake"
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   COMPONENT Devel
   )

install (
   FILES ${kactivities_version_header}
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KActivities COMPONENT Devel
   )

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)


kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
