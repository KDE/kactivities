project (activitymanager-uihandler-kdialog)

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
    uihandler_kdialog_SRCS
    KDialogUi.cpp
    ../../UiHandler.cpp
    )

kde4_add_plugin (
    activitymanager_uihandler_kdialog
    ${uihandler_kdialog_SRCS}
    )

target_link_libraries (
    activitymanager_uihandler_kdialog
    ${KDE4_KDEUI_LIBS}
    )

install (TARGETS activitymanager_uihandler_kdialog DESTINATION ${PLUGIN_INSTALL_DIR})

