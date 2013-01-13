project (kactivities-models)

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

# Library API version
set (KACTIVITIES_MODELS_LIB_MAJOR_VERSION "1")
set (KACTIVITIES_MODELS_LIB_MINOR_VERSION "0")
set (KACTIVITIES_MODELS_LIB_PATCH_VERSION "0")

# Suffix to add at end of version string. Usual values are:
# "-git"   : alpha code unstable from git. Do not use in production
# "-beta1" : beta1 release.
# "-beta2" : beta2 release.
# "-beta3" : beta3 release.
# "-rc"    : release candidate.
# ""       : final relase. Can be used in production.
set (KACTIVITIES_MODELS_LIB_SUFFIX_VERSION "-git")

# Library ABI version used by linker.
# For details : http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
set (KACTIVITIES_MODELS_LIB_SO_CUR_VERSION "1")
set (KACTIVITIES_MODELS_LIB_SO_REV_VERSION "0")
set (KACTIVITIES_MODELS_LIB_SO_AGE_VERSION "0")

# =======================================================
# Set env. variables accordinly.

set (KACTIVITIES_MODELS_INCLUDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/"
    CACHE STRING
    "Location of libkactivities-models headers" FORCE)
set (KACTIVITIES_MODELS_LIBS
    "kactivities-models"
    CACHE STRING
    "Location of libkactivities-models binary" FORCE)

set (KACTIVITIES_MODELS_LIB_VERSION_STRING "${KACTIVITIES_MODELS_LIB_MAJOR_VERSION}.${KACTIVITIES_MODELS_LIB_MINOR_VERSION}.${KACTIVITIES_MODELS_LIB_PATCH_VERSION}${KACTIVITIES_MODELS_LIB_SUFFIX_VERSION}")
set (KACTIVITIES_MODELS_LIB_VERSION_ID "0x0${KACTIVITIES_MODELS_LIB_MAJOR_VERSION}0${KACTIVITIES_MODELS_LIB_MINOR_VERSION}0${KACTIVITIES_MODELS_LIB_PATCH_VERSION}")
set (KACTIVITIES_MODELS_LIB_SO_VERSION_STRING "${KACTIVITIES_MODELS_LIB_SO_CUR_VERSION}.${KACTIVITIES_MODELS_LIB_SO_REV_VERSION}.${KACTIVITIES_MODELS_LIB_SO_AGE_VERSION}")

add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${QDBUS_INCLUDE_DIRS}
    ${KDE4_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    ${NEPOMUK_CORE_INCLUDE_DIR}
    ${SOPRANO_INCLUDE_DIR}
    )

set (
    kactivities-models_LIB_SRCS

    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.cpp
    activitymodel.cpp
    resourcemodel.cpp
    ../core/manager_p.cpp
    )

set (
    kactivities-models-component_LIB_SRCS
    activitiescomponentplugin.cpp
    )

set_source_files_properties (
    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
    PROPERTIES
    INCLUDE ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.h
    )

qt4_add_dbus_interface (
    kactivities-models_LIB_SRCS

    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
    activities_interface
    )

qt4_add_dbus_interface (
    kactivities-models_LIB_SRCS

    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Resources.xml
    resources_interface
    )

qt4_add_dbus_interface (
    kactivities-models_LIB_SRCS

    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Features.xml
    features_interface
    )

qt4_add_dbus_interface (
    kactivities-models_LIB_SRCS

    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
    resources_linking_interface
    )

kde4_add_library (
    kactivities-models SHARED
    ${kactivities-models_LIB_SRCS}
    )

kde4_add_library (
    kactivities-models-component-plugin SHARED
    ${kactivities-models-component_LIB_SRCS}
    )

set_target_properties (
    kactivities-models
    PROPERTIES
    VERSION ${KACTIVITIES_MODELS_LIB_SO_VERSION_STRING}
    SOVERSION ${KACTIVITIES_MODELS_LIB_SO_CUR_VERSION}
    )

target_link_libraries (
    kactivities-models
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${NEPOMUK_CORE_LIBRARY}
    ${SOPRANO_LIBRARIES}
    )

target_link_libraries (
    kactivities-models-component-plugin
    kactivities-models
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${NEPOMUK_CORE_LIBRARY}
    ${SOPRANO_LIBRARIES}
    ${QT_QTDECLARATIVE_LIBRARY}
    )

## install

set (
    kactivities-models_LIB_HEADERS

    activitymodel.h
    resourcemodel.h
    kactivities_models_export.h
    )

set (
    kactivities-models_LIB_PRETTY_HEADERS
    includes/KActivities/Models/ActivityModel
    includes/KActivities/Models/ResourceModel
    )

install (
    FILES ${kactivities-models_LIB_HEADERS}
    DESTINATION ${INCLUDE_INSTALL_DIR}/kactivities-models
    COMPONENT Devel
    )

install (
    FILES ${kactivities-models_LIB_PRETTY_HEADERS}
    DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/KActivities/Models
    COMPONENT Devel
    )

install (
    TARGETS kactivities-models
    EXPORT kactivities-modelsLibraryTargets
    ${INSTALL_TARGETS_DEFAULT_ARGS}
    )

install (
    EXPORT kactivities-modelsLibraryTargets
    DESTINATION ${LIB_INSTALL_DIR}/cmake/KActivities-Models
    FILE KActivitiesModelsLibraryTargets.cmake
    )

install (
    TARGETS
    kactivities-models-component-plugin
    DESTINATION
    ${IMPORTS_INSTALL_DIR}/org/kde/activities/models
    )

install (
    FILES
    qmldir
    DESTINATION
    ${IMPORTS_INSTALL_DIR}/org/kde/activities/models
    )

if (NOT WIN32)
    configure_file (${CMAKE_CURRENT_SOURCE_DIR}/libkactivities-models.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libkactivities-models.pc)
    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/libkactivities-models.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig )
endif (NOT WIN32)

configure_file (KActivities-ModelsConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KActivities-ModelsConfig.cmake @ONLY)

macro_write_basic_cmake_version_file (
    ${CMAKE_CURRENT_BINARY_DIR}/KActivities-ModelsConfigVersion.cmake
    ${KACTIVITIES_MODELS_LIB_MAJOR_VERSION}
    ${KACTIVITIES_MODELS_LIB_MINOR_VERSION}
    ${KACTIVITIES_MODELS_LIB_PATCH_VERSION}
    )

install (
    FILES ${CMAKE_CURRENT_BINARY_DIR}/KActivities-ModelsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/KActivities-ModelsConfigVersion.cmake
    DESTINATION ${LIB_INSTALL_DIR}/cmake/KActivities-Models
    )

