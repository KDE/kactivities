# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivities-imports)
find_package (ECM 0.0.8 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Sql Gui Widgets Qml)
find_package (KF5 CONFIG REQUIRED KI18n KDBusAddons)

set (
   kactivities_imports_LIB_SRCS
   activitiesextensionplugin.cpp
   activitymodel.cpp
#   resourcemodel.cpp
   )

add_library (kactivitiesextensionplugin SHARED ${kactivities_imports_LIB_SRCS})

target_link_libraries (
   kactivitiesextensionplugin
   Qt5::Core
   Qt5::DBus
   Qt5::Gui
   Qt5::Qml
   KF5::KI18n
   KF5::KDBusAddons
   KActivities
   )

## install

install (TARGETS kactivitiesextensionplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/activities)
install (FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/activities)

