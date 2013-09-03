# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (ActivityManager)

# General
# find_package(KDeclarative)
# set (ADDITIONAL_LINK_LIBS)
# set (sdo_SRCS)

# Standard stuff

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (plugin_implementation_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/Plugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/Module.cpp
   )

# add_subdirectory (plugins)
include_directories (
   ${KDBusAddons_INCLUDE_DIR}
   ${KConfig_INCLUDE_DIR}
   ${KCoreAddons_INCLUDE_DIR}
   ${KWindowSystem_INCLUDE_DIR}
   )

set (activity_manager_SRCS
   Application.cpp
   ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.cpp

   Activities.cpp
   Resources.cpp
   Features.cpp

   ${plugin_implementation_SRCS}

   Event.cpp

   jobs/Job.cpp
   jobs/JobFactory.cpp

   jobs/schedulers/Abstract.cpp
   jobs/schedulers/Ordered.cpp
   jobs/schedulers/Fallible.cpp
   jobs/schedulers/Given.cpp
   jobs/schedulers/Retry.cpp
   jobs/schedulers/Switch.cpp
   jobs/schedulers/Test.cpp

   jobs/general/Call.cpp

   jobs/ksmserver/KSMServer.cpp
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
   KF5::KDBusAddons
   KF5::KCoreAddons
   KF5::KConfigCore
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

