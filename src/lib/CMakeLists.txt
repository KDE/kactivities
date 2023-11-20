# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

# =======================================================
# Now that we finished with the boilerplate, start
# with the library definition

add_library(KF6Activities)
add_library(KF6::Activities ALIAS KF6Activities)

set_target_properties(KF6Activities PROPERTIES
   VERSION     ${KACTIVITIES_VERSION}
   SOVERSION   ${KACTIVITIES_SOVERSION}
   EXPORT_NAME Activities
)

target_sources(KF6Activities PRIVATE
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

set(KActivities_DBus_SRCS)
qt_add_dbus_interface(KActivities_DBus_SRCS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
   activities_interface
)
qt_add_dbus_interface(KActivities_DBus_SRCS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Resources.xml
   resources_interface
)
qt_add_dbus_interface(KActivities_DBus_SRCS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Features.xml
   features_interface
)
qt_add_dbus_interface(KActivities_DBus_SRCS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   resources_linking_interface
)
qt_add_dbus_interface(KActivities_DBus_SRCS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Application.xml
   application_interface
)
target_sources(KF6Activities PRIVATE
   ${KActivities_DBus_SRCS}
)

ecm_qt_declare_logging_category(KF6Activities
    HEADER debug_p.h
    IDENTIFIER KAMD_CORELIB
    CATEGORY_NAME kf.activities
    OLD_CATEGORY_NAMES org.kde.kactivities.lib.core
    DEFAULT_SEVERITY Warning
    DESCRIPTION "kactivities core lib"
    EXPORT KACTIVITIES
)

target_link_libraries(KF6Activities
   PUBLIC
      Qt6::Core
   PRIVATE
      Qt6::DBus
)

set(KACTIVITIES_BUILD_INCLUDE_DIRS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src
   ${CMAKE_BINARY_DIR}/
)

target_include_directories(KF6Activities
   INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KActivities>"
   PRIVATE  ${KACTIVITIES_BUILD_INCLUDE_DIRS}
)

# install
ecm_generate_export_header (KF6Activities
   BASE_NAME KActivities
   GROUP_BASE_NAME KF
   VERSION ${KF6_MIN_VERSION}
   USE_VERSION_HEADER
   DEPRECATED_BASE_VERSION 0
)

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
   TARGETS KF6Activities
   EXPORT KF6ActivitiesLibraryTargets
   ${KF_INSTALL_TARGETS_DEFAULT_ARGS}
   )

if(BUILD_QCH)
    ecm_add_qch(
        KF6Activities_QCH
        NAME KActivities
        BASE_NAME KF6Activities
        VERSION ${PROJECT_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KActivities_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
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
    ecm_generate_pkgconfig_file(BASE_NAME KF6Activities
      LIB_NAME KF6Activities
      INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KActivities
      DEPS Qt6Core
      DESCRIPTION "libKActivities is a C++ library for using KDE activities"
      INSTALL
    )
endif ()

