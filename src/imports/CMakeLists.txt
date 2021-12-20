# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivities-imports)

find_package (Qt${QT_MAJOR_VERSION} REQUIRED NO_MODULE COMPONENTS Gui Qml Quick Sql)
find_package (KF5Config     ${KF_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5CoreAddons ${KF_DEP_VERSION} CONFIG REQUIRED)

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
   Qt${QT_MAJOR_VERSION}::Core
   Qt${QT_MAJOR_VERSION}::DBus
   Qt${QT_MAJOR_VERSION}::Gui
   Qt${QT_MAJOR_VERSION}::Qml
   Qt${QT_MAJOR_VERSION}::Quick
   Qt${QT_MAJOR_VERSION}::Sql
   KF5::Activities
   KF5::ConfigCore
   KF5::CoreAddons
   )

## install

install (TARGETS kactivitiesextensionplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/activities)
install (FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/activities)

