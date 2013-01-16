project (activitymanager-plugin-nepomuk)

set (
    nepomukplugin_SRCS
    NepomukPlugin.cpp
    NepomukCommon.cpp
    ${plugin_implementation_SRCS}
    )

qt4_add_dbus_adaptor (
    nepomukplugin_SRCS
    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
    NepomukPlugin.h NepomukPlugin
    )

soprano_add_ontology (
    nepomukplugin_SRCS
    ${CMAKE_SOURCE_DIR}/src/ontologies/kao.trig
    "KAO" "KDE::Vocabulary" "trig"
    )

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/../..
    ${KDE4_INCLUDES}
    ${SOPRANO_INCLUDE_DIR}
    ${NEPOMUK_CORE_INCLUDE_DIR}
    )

kde4_add_plugin (
    activitymanager_plugin_nepomuk
    ${nepomukplugin_SRCS}
    )

target_link_libraries (
    activitymanager_plugin_nepomuk
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${NEPOMUK_CORE_LIBRARY}
    ${SOPRANO_LIBRARIES}
    )

install (TARGETS activitymanager_plugin_nepomuk DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES activitymanager-plugin-nepomuk.desktop DESTINATION ${SERVICES_INSTALL_DIR})

