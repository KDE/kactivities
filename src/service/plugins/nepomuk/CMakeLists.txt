project (activitymanager-plugin-nepomuk)

set (
    nepomukplugin_SRCS
    NepomukPlugin.cpp
    NepomukCommon.cpp
    ${plugin_implementation_SRCS}
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
    )

kde4_add_plugin (
    activitymanager_plugin_nepomuk
    ${nepomukplugin_SRCS}
    )

target_link_libraries (
    activitymanager_plugin_nepomuk
    ${QT_QTSQL_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${NEPOMUK_CORE_LIBRARY}
    ${SOPRANO_LIBRARIES}
    )

install (TARGETS activitymanager_plugin_nepomuk DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES activitymanager-plugin-nepomuk.desktop DESTINATION ${SERVICES_INSTALL_DIR})

