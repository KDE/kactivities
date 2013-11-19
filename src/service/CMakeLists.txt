# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (ActivityManager)

# General
find_package (ECM 0.0.8 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Sql Gui Widgets)

find_package (KF5 CONFIG REQUIRED KWindowSystem KCoreAddons KConfig KI18n)

# Standard stuff

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (plugin_implementation_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/Plugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/Module.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/Event.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/Debug.cpp
   )

add_subdirectory (plugins)

set (activity_manager_SRCS
   Application.cpp
   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.cpp

   Activities.cpp
   Resources.cpp
   Features.cpp

   ${plugin_implementation_SRCS}

   Event.cpp

   ksmserver/KSMServer.cpp
   )

qt5_add_dbus_adaptor (
   activity_manager_SRCS
   ../common/dbus/org.kde.ActivityManager.Activities.xml
   Activities.h Activities
   )

qt5_add_dbus_adaptor (
   activity_manager_SRCS
   ../common/dbus/org.kde.ActivityManager.Resources.xml
   Resources.h Resources
   )

qt5_add_dbus_adaptor (
   activity_manager_SRCS
   ../common/dbus/org.kde.ActivityManager.Features.xml
   Features.h Features
   )

add_executable (activity-manager ${activity_manager_SRCS})

target_link_libraries (activity-manager
   Qt5::Core
   Qt5::DBus
   Qt5::Widgets
   KF5::KDBusAddons
   KF5::KCoreAddons
   KF5::KConfigCore
   KF5::KI18n
   KF5::KWindowSystem
   )

set_target_properties (activity-manager
   PROPERTIES OUTPUT_NAME kactivitymanagerd
   )

########### install application ###############

install (FILES
   files/kactivitymanagerd.desktop
   DESTINATION ${SERVICES_INSTALL_DIR}
   )

install (TARGETS
   activity-manager ${INSTALL_TARGETS_DEFAULT_ARGS}
   )

install (FILES
   files/activitymanager-plugin.desktop
   DESTINATION ${SERVICETYPES_INSTALL_DIR}
   )

