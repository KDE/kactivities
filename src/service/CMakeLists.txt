# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (ActivityManager)

# General
find_package (ECM 0.0.8 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Sql Gui Widgets)

find_package (KF5Config ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5CoreAddons ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5I18n ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5Service ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5WindowSystem ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5GlobalAccel ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5XmlGui ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5KIO ${KF5_DEP_VERSION} CONFIG REQUIRED)

# Standard stuff

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (plugin_implementation_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/Plugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/Module.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/Event.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/Debug.cpp
   )

add_subdirectory (plugins)

set (kactivitymanager_SRCS
   Application.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.cpp

   Activities.cpp
   Resources.cpp
   Features.cpp

   ${plugin_implementation_SRCS}

   Event.cpp

   ksmserver/KSMServer.cpp
   )

qt5_add_dbus_adaptor (
   kactivitymanager_SRCS
   ../common/dbus/org.kde.ActivityManager.Activities.xml
   Activities.h Activities
   )

qt5_add_dbus_adaptor (
   kactivitymanager_SRCS
   ../common/dbus/org.kde.ActivityManager.Resources.xml
   Resources.h Resources
   )

qt5_add_dbus_adaptor (
   kactivitymanager_SRCS
   ../common/dbus/org.kde.ActivityManager.Features.xml
   Features.h Features
   )

add_executable (kactivitymanagerd ${kactivitymanager_SRCS})

target_link_libraries (kactivitymanagerd
   Qt5::Core
   Qt5::DBus
   Qt5::Widgets
   KF5::DBusAddons
   KF5::CoreAddons
   KF5::ConfigCore
   KF5::I18n
   KF5::Service
   KF5::WindowSystem
   )

########### install application ###############

install (FILES
   files/kactivitymanagerd.desktop
   DESTINATION ${KDE_INSTALL_KSERVICES5DIR}
   )

install (TARGETS
   kactivitymanagerd ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
   )

install (FILES
   files/kactivitymanagerd-plugin.desktop
   DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR}
   )

