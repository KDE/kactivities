# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

project (kactivities-imports)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Gui Qml Quick Sql)
find_package (KF6Config     ${KF_DEP_VERSION} CONFIG REQUIRED)
find_package (KF6CoreAddons ${KF_DEP_VERSION} CONFIG REQUIRED)

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
   kactivitiesextensionplugin PRIVATE
   Qt6::Core
   Qt6::DBus
   Qt6::Gui
   Qt6::Qml
   Qt6::Quick
   Qt6::Sql
   KF6::Activities
   KF6::ConfigCore
   KF6::CoreAddons
   Boost::headers
)

ecm_finalize_qml_module(kactivitiesextensionplugin DESTINATION ${KDE_INSTALL_QMLDIR})
