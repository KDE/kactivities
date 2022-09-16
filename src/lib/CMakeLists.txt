# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

# =======================================================
# Now that we finished with the boilerplate, start
# with the library definition

set (
   KActivities_LIB_SRCS

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.cpp

   consumer.cpp
   controller.cpp
   info.cpp
   resourceinstance.cpp
   activitiesmodel.cpp

   mainthreadexecutor_p.cpp
   manager_p.cpp
   activitiescache_p.cpp

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/utils/dbusfuture_p.cpp

   version.cpp
   )

set_source_files_properties (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   PROPERTIES
   INCLUDE ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.h
   )

qt_add_dbus_interface (
   KActivities_LIB_SRCS

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   activities_interface
   )

qt_add_dbus_interface (
   KActivities_LIB_SRCS

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Resources.xml
   resources_interface
   )

qt_add_dbus_interface (
   KActivities_LIB_SRCS

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Features.xml
   features_interface
   )

qt_add_dbus_interface (
   KActivities_LIB_SRCS

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   resources_linking_interface
   )

qt_add_dbus_interface (
   KActivities_LIB_SRCS

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Application.xml
   application_interface
   )

ecm_qt_declare_logging_category(KActivities_LIB_SRCS
    HEADER debug_p.h
    IDENTIFIER KAMD_CORELIB
    CATEGORY_NAME kf.activities
    OLD_CATEGORY_NAMES org.kde.kactivities.lib.core
    DEFAULT_SEVERITY Warning
    DESCRIPTION "kactivities core lib"
    EXPORT KACTIVITIES
)


add_library (
   KF5Activities SHARED
   ${KActivities_LIB_SRCS}
   )
add_library (KF5::Activities ALIAS KF5Activities)

set(KACTIVITIES_BUILD_INCLUDE_DIRS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/
   )
include_directories (${KACTIVITIES_BUILD_INCLUDE_DIRS})

set_target_properties (
   KF5Activities
   PROPERTIES
   VERSION ${KACTIVITIES_VERSION}
   SOVERSION ${KACTIVITIES_SOVERSION}
   EXPORT_NAME Activities
   )

target_link_libraries (
   KF5Activities
   PUBLIC
      Qt${QT_MAJOR_VERSION}::Core
   PRIVATE
      Qt${QT_MAJOR_VERSION}::DBus
   )

target_include_directories (
   KF5Activities
   INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KActivities>"
   )

# install
generate_export_header (KF5Activities BASE_NAME KActivities)

ecm_generate_headers (
   KActivities_CamelCase_HEADERS
   HEADER_NAMES
   Consumer
   Controller
   Info
   ResourceInstance
   ActivitiesModel
   Version
   PREFIX KActivities
   REQUIRED_HEADERS KActivities_HEADERS
   )
install (
   FILES ${KActivities_CamelCase_HEADERS}
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KActivities/KActivities
   COMPONENT Devel
   )

install (
   FILES ${KActivities_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/kactivities_export.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KActivities/kactivities
   COMPONENT Devel
   )

install (
   TARGETS KF5Activities
   EXPORT KF5ActivitiesLibraryTargets
   ${KF_INSTALL_TARGETS_DEFAULT_ARGS}
   )

if(BUILD_QCH)
    ecm_add_qch(
        KF5Activities_QCH
        NAME KActivities
        BASE_NAME KF5Activities
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KActivities_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Core_QCH
        INCLUDE_DIRS
            ${KACTIVITIES_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            KACTIVITIES_EXPORT
            KACTIVITIES_DEPRECATED
            KACTIVITIES_DEPRECATED_EXPORT
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()


if (NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME libKActivities
      LIB_NAME KF5Activities
      INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KActivities
      DEPS Qt${QT_MAJOR_VERSION}Core
      DESCRIPTION "libKActivities is a C++ library for using KDE activities"
      INSTALL
    )
endif ()

include (ECMGeneratePriFile)
ecm_generate_pri_file (
   BASE_NAME KActivities
   LIB_NAME KF5Activities
   FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KActivities
   )
install (
   FILES ${PRI_FILENAME}
   DESTINATION ${ECM_MKSPECS_INSTALL_DIR}
   )

