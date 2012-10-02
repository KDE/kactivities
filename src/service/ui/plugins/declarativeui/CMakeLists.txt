project (activitymanager-uihandler-declarative)

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${KDE4_KDEUI_INCLUDES}
    )

set (
    uihandler_declarative_SRCS
    DeclarativeUi.cpp
    DataEngineBindings.cpp
    KDeclarativeMainWindow.cpp
    KDeclarativeView.cpp
    ../../UiHandler.cpp
    )

install (DIRECTORY package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.ActivityManager.UiHandler)

kde4_add_plugin (
    activitymanager_uihandler_declarative
    ${uihandler_declarative_SRCS}
    )

target_link_libraries (
    activitymanager_uihandler_declarative
    ${QT_QTDECLARATIVE_LIBRARY}
    ${QT_QTOPENGL_LIBRARY}
    ${QT_QTSCRIPT_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${X11_LIBRARIES}
    kdeclarative
    )

install (TARGETS activitymanager_uihandler_declarative DESTINATION ${PLUGIN_INSTALL_DIR})

