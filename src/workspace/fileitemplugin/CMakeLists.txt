include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    )


set (kactivitymanagerd_fileitem_linking_plugin_SRCS
    FileItemLinkingPlugin.cpp
    )

kde4_add_plugin (kactivitymanagerd_fileitem_linking_plugin ${kactivitymanagerd_fileitem_linking_plugin_SRCS})

target_link_libraries (
    kactivitymanagerd_fileitem_linking_plugin
    ${KDE4_KIO_LIBS}
    kactivities
    )

install (TARGETS kactivitymanagerd_fileitem_linking_plugin DESTINATION ${PLUGIN_INSTALL_DIR})
install (FILES kactivitymanagerd_fileitem_linking_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
