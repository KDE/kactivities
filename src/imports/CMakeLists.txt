# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivities-imports)

find_package (Qt${QT_MAJOR_VERSION} REQUIRED NO_MODULE COMPONENTS Gui Qml Quick Sql)
find_package (KF5Config     ${KF_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5CoreAddons ${KF_DEP_VERSION} CONFIG REQUIRED)

ecm_add_qml_module(kactivitiesextensionplugin URI "org.kde.activities" VERSION 0.1)

target_sources(kactivitiesextensionplugin PRIVATE
   activitiesextensionplugin.cpp
   activitymodel.cpp
   activityinfo.cpp
#  resourcemodel.cpp
   resourceinstance.cpp

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/utils/dbusfuture_p.cpp
)

target_link_libraries(
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

ecm_finalize_qml_module(kactivitiesextensionplugin DESTINATION ${KDE_INSTALL_QMLDIR})
