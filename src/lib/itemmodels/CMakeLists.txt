project(kactivities-itemmodels)

FIND_PACKAGE(KDE4 REQUIRED)
INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)
INCLUDE(MacroOptionalAddSubdirectory)
INCLUDE(FindPackageHandleStandardArgs)

# =======================================================
# Information to update before to release this library.

# Library version history:
# API      ABI
# 0.1.0 => 0.1.0
# 0.1.1 => 0.1.1
# 0.2.0 => 0.2.0

# Library API version
SET(KACTIVITIES_ITEMMODELS_LIB_MAJOR_VERSION "1")
SET(KACTIVITIES_ITEMMODELS_LIB_MINOR_VERSION "0")
SET(KACTIVITIES_ITEMMODELS_LIB_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final relase. Can be used in production.
SET(KACTIVITIES_ITEMMODELS_LIB_SUFFIX_VERSION "-git")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
SET(KACTIVITIES_ITEMMODELS_LIB_SO_CUR_VERSION "1")
SET(KACTIVITIES_ITEMMODELS_LIB_SO_REV_VERSION "0")
SET(KACTIVITIES_ITEMMODELS_LIB_SO_AGE_VERSION "0")

# =======================================================
# Set env. variables accordinly.

set(KACTIVITIES_ITEMMODELS_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/"
    CACHE STRING
    "Location of libkactivities-itemmodels headers" FORCE)
set(KACTIVITIES_ITEMMODELS_LIBS
    "kactivities-itemmodels"
    CACHE STRING
    "Location of libkactivities-itemmodels binary" FORCE)

SET(KACTIVITIES_ITEMMODELS_LIB_VERSION_STRING "${KACTIVITIES_ITEMMODELS_LIB_MAJOR_VERSION}.${KACTIVITIES_ITEMMODELS_LIB_MINOR_VERSION}.${KACTIVITIES_ITEMMODELS_LIB_PATCH_VERSION}${KACTIVITIES_ITEMMODELS_LIB_SUFFIX_VERSION}")
SET(KACTIVITIES_ITEMMODELS_LIB_VERSION_ID "0x0${KACTIVITIES_ITEMMODELS_LIB_MAJOR_VERSION}0${KACTIVITIES_ITEMMODELS_LIB_MINOR_VERSION}0${KACTIVITIES_ITEMMODELS_LIB_PATCH_VERSION}")
SET(KACTIVITIES_ITEMMODELS_LIB_SO_VERSION_STRING "${KACTIVITIES_ITEMMODELS_LIB_SO_CUR_VERSION}.${KACTIVITIES_ITEMMODELS_LIB_SO_REV_VERSION}.${KACTIVITIES_ITEMMODELS_LIB_SO_AGE_VERSION}")

ADD_DEFINITIONS (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
   ${CMAKE_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/src
   ${QDBUS_INCLUDE_DIRS}
   ${KDE4_INCLUDES}
   ${KDE4_KIO_INCLUDES}
   )

set(
   kactivities-itemmodels_LIB_SRCS

   activities.cpp
   )

qt4_add_dbus_interface(
   kactivities-itemmodels_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   activities_interface
   )

qt4_add_dbus_interface(
   kactivities-itemmodels_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Resources.xml
   resources_interface
   )

qt4_add_dbus_interface(
   kactivities-itemmodels_LIB_SRCS

   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Features.xml
   features_interface
   )

kde4_add_library(
   kactivities-itemmodels SHARED
   ${kactivities-itemmodels_LIB_SRCS}
   )


set_target_properties(
   kactivities-itemmodels
   PROPERTIES
   VERSION ${KACTIVITIES_ITEMMODELS_LIB_SO_VERSION_STRING}
   SOVERSION ${KACTIVITIES_ITEMMODELS_LIB_SO_CUR_VERSION}
   )

target_link_libraries(
   kactivities-itemmodels
   ${KDE4_KDECORE_LIBS}
   )

## install

set(
   kactivities-itemmodels_LIB_HEADERS

   activities.h
   kactivities_itemmodels_export.h
   )

set(
   kactivities-itemmodels_LIB_PRETTY_HEADERS
   includes/KActivities/ItemModels/Activities
   )

install(
   FILES ${kactivities-itemmodels_LIB_HEADERS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/kactivities-itemmodels
   COMPONENT Devel
   )

install(
   FILES ${kactivities-itemmodels_LIB_PRETTY_HEADERS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KActivities/ItemModels
   COMPONENT Devel
   )

install(
   TARGETS kactivities-itemmodels
   EXPORT kactivities-itemmodelsLibraryTargets
   ${INSTALL_TARGETS_DEFAULT_ARGS}
   )

install(
   EXPORT kactivities-itemmodelsLibraryTargets
   DESTINATION ${LIB_INSTALL_DIR}/cmake/KActivities/ItemModels
   FILE KActivities-ItemModelsLibraryTargets.cmake
   )

IF(NOT WIN32)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libkactivities-itemmodels.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libkactivities-itemmodels.pc)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libkactivities-itemmodels.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
ENDIF(NOT WIN32)

CONFIGURE_FILE(KActivities-ItemModelsConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KActivities-ItemModelsConfig.cmake @ONLY)
MACRO_WRITE_BASIC_CMAKE_VERSION_FILE(${CMAKE_CURRENT_BINARY_DIR}/KActivities-ItemModelsConfigVersion.cmake
                                     ${KACTIVITIES_ITEMMODELS_LIB_MAJOR_VERSION} ${KACTIVITIES_ITEMMODELS_LIB_MINOR_VERSION} ${KACTIVITIES_ITEMMODELS_LIB_PATCH_VERSION})

INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/KActivities-ItemModelsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/KActivities-ItemModelsConfigVersion.cmake
    DESTINATION ${LIB_INSTALL_DIR}/cmake/KActivities-ItemModels
)

