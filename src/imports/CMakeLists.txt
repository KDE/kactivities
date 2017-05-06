# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivities-imports)
find_package (ECM 0.0.8 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Sql Gui Qml Quick Sql)
find_package (KF5Config     ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5CoreAddons ${KF5_DEP_VERSION} CONFIG REQUIRED)

set (
   kactivities_imports_LIB_SRCS
   activitiesextensionplugin.cpp
   activitymodel.cpp
   activityinfo.cpp
#  resourcemodel.cpp
   resourceinstance.cpp

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/utils/dbusfuture_p.cpp
   )

add_library (kactivitiesextensionplugin SHARED ${kactivities_imports_LIB_SRCS})

target_link_libraries (
   kactivitiesextensionplugin
   Qt5::Core
   Qt5::DBus
   Qt5::Gui
   Qt5::Qml
   Qt5::Quick
   Qt5::Sql
   KF5::Activities
   KF5::ConfigCore
   KF5::CoreAddons
   )

## install

install (TARGETS kactivitiesextensionplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/activities)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/activities)

