# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KCMActivities)

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core Quick Sql Qml DBus)
find_package (KF5KCMUtils ${KF5_DEP_VERSION} CONFIG REQUIRED)
find_package (KF5Declarative ${KF5_DEP_VERSION} CONFIG REQUIRED)

set (KAMD_KCM_SRCS
   kcm_activities.cpp
   MainConfigurationWidget.cpp
   BlacklistedApplicationsModel.cpp
   )

ki18n_wrap_ui (
   KAMD_KCM_SRCS
   ui/MainConfigurationWidgetBase.ui
   )

add_library (kcm_activities MODULE ${KAMD_KCM_SRCS})

target_link_libraries (kcm_activities
   Qt5::Quick
   Qt5::Core
   Qt5::Sql
   Qt5::Qml
   Qt5::DBus
   KF5::KCMUtils
   KF5::I18n
   KF5::Service
   KF5::Declarative
   )

install (
   TARGETS kcm_activities
   DESTINATION ${PLUGIN_INSTALL_DIR}
   )

install (
   FILES kcm_activities.desktop
   DESTINATION ${SERVICES_INSTALL_DIR}
   )

install (
   FILES qml/BlacklistApplicationView.qml
   DESTINATION ${KF5_DATA_INSTALL_DIR}/kactivitymanagerd/workspace/settings
   )
