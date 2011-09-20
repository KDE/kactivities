project(activitymanager-dummy)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(
   dummy_SRCS
   dummy.cpp
   ../../Plugin.cpp
   )

kde4_add_plugin(
   activitymanager_plugin_dummy
   ${dummy_SRCS}
   )

target_link_libraries(
   activitymanager_plugin_dummy
   ${KDE4_KDECORE_LIBS}
)

install(TARGETS activitymanager_plugin_dummy DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES activitymanager-plugin-dummy.desktop DESTINATION ${SERVICES_INSTALL_DIR})

