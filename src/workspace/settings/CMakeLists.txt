project (KCMActivities)

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${KDECLARATIVE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/
)

set (KAMD_KCM_SRCS
    kcm_activities.cpp
    MainConfigurationWidget.cpp
    BlacklistedApplicationsModel.cpp
)

kde4_add_ui_files (
    KAMD_KCM_SRCS
    ui/MainConfigurationWidgetBase.ui
)

kde4_add_plugin (kcm_activities ${KAMD_KCM_SRCS})

target_link_libraries (kcm_activities
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    ${QT_QTSQL_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${QT_QTOPENGL_LIBRARY}
    ${KDECLARATIVE_LIBRARIES}
    ${KDE4_PLASMA_LIBS}
)

install (TARGETS kcm_activities
    DESTINATION ${PLUGIN_INSTALL_DIR}
)

install (FILES kcm_activities.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)

install (FILES
    qml/BlacklistApplicationView.qml
    DESTINATION ${DATA_INSTALL_DIR}/activitymanager/workspace/settings
)
