project (kactivities)

find_package (Qt4 REQUIRED)
find_package (KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)
include (MacroOptionalAddSubdirectory)
include (FindPackageHandleStandardArgs)

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

add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${QDBUS_INCLUDE_DIRS}
    ${KDE4_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    )

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

qt4_add_dbus_interface (
    kactivities_LIB_SRCS

    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
    activities_interface
    )

qt4_add_dbus_interface (
    kactivities_LIB_SRCS

    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Resources.xml
    resources_interface
    )

qt4_add_dbus_interface (
    kactivities_LIB_SRCS

    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Features.xml
    features_interface
    )

qt4_add_dbus_interface (
    kactivities_LIB_SRCS

    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
    resources_linking_interface
    )

kde4_add_library (
    kactivities SHARED
    ${kactivities_LIB_SRCS}
    )

set_target_properties (
    kactivities
    PROPERTIES
    VERSION ${KACTIVITIES_LIB_SO_VERSION_STRING}
    SOVERSION ${KACTIVITIES_LIB_SO_CUR_VERSION}
    )

target_link_libraries (
    kactivities
    ${KDE4_KDECORE_LIBS}
    )

## install

set (
    kactivities_LIB_HEADERS
    consumer.h
    controller.h
    info.h
    resourceinstance.h
    version.h
    kactivities_export.h
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
    TARGETS kactivities
    EXPORT kactivitiesLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
    )

install (
    EXPORT kactivitiesLibraryTargets
    DESTINATION ${LIB_INSTALL_DIR}/cmake/KActivities
    FILE KActivitiesLibraryTargets.cmake
    )

if (NOT WIN32)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/libkactivities.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libkactivities.pc)
    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/libkactivities.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
endif ()

configure_file (KActivitiesConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KActivitiesConfig.cmake @ONLY)
macro_write_basic_cmake_version_file (
    ${CMAKE_CURRENT_BINARY_DIR}/KActivitiesConfigVersion.cmake
    ${KACTIVITIES_LIB_MAJOR_VERSION}
    ${KACTIVITIES_LIB_MINOR_VERSION}
    ${KACTIVITIES_LIB_PATCH_VERSION}
    )

install (
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/KActivitiesConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/KActivitiesConfigVersion.cmake
    DESTINATION
    ${LIB_INSTALL_DIR}/cmake/KActivities
)

