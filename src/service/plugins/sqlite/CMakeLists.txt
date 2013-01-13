project (activitymanager-plugin-sqlite)

set (
    sqliteplugin_SRCS
    DatabaseConnection.cpp
    StatsPlugin.cpp
    ResourceScoreCache.cpp
    ResourceScoreMaintainer.cpp
    ${plugin_implementation_SRCS}
    )

qt4_add_dbus_adaptor (
    sqliteplugin_SRCS
    org.kde.ActivityManager.Resources.Scoring.xml
    StatsPlugin.h StatsPlugin
    )

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/../..
    ${KDE4_INCLUDES}
    )

kde4_add_plugin (
    activitymanager_plugin_sqlite
    ${sqliteplugin_SRCS}
    )

target_link_libraries (
    activitymanager_plugin_sqlite
    ${QT_QTSQL_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    )

install (TARGETS activitymanager_plugin_sqlite DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES activitymanager-plugin-sqlite.desktop DESTINATION ${SERVICES_INSTALL_DIR})

