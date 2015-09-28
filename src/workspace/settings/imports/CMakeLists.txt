# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivities-settings)
find_package (ECM 0.0.8 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Sql Gui Widgets Qml Quick Sql QuickWidgets)
# find_package (KF5DBusAddons ${KF5_DEP_VERSION} CONFIG REQUIRED)
# find_package (KF5I18n       ${KF5_DEP_VERSION} CONFIG REQUIRED)
# find_package (KF5Config     ${KF5_DEP_VERSION} CONFIG REQUIRED)
# find_package (KF5CoreAddons ${KF5_DEP_VERSION} CONFIG REQUIRED)
# find_package (KF5KIO        ${KF5_DEP_VERSION} CONFIG REQUIRED)

set (
   kactivities_imports_LIB_SRCS
   plugin.cpp
   activitysettings.cpp
   dialog.cpp
   )

add_library (kactivitiessettingsplugin SHARED ${kactivities_imports_LIB_SRCS})

target_link_libraries (
   kactivitiessettingsplugin
   Qt5::Core
   Qt5::DBus
   Qt5::Gui
   Qt5::Qml
   Qt5::Quick
   Qt5::Widgets
   Qt5::QuickWidgets
   KF5::I18n
   )

## install

install (
   DIRECTORY
   qml/activityDialog

   DESTINATION ${KDE_INSTALL_DATADIR_KF5}/kactivitymanagerd/workspace/settings/qml/
   )

install (TARGETS kactivitiessettingsplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/activities/settings)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/activities/settings)

