project(activitymanager-plugin-slc)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

set(
   slc_SRCS
   slc.cpp
   ../../Plugin.cpp
   ../../SharedInfo.cpp
   )

qt4_add_dbus_adaptor(
   slc_SRCS org.kde.ActivityManager.SLC.xml
   slc.h SlcPlugin
   )

kde4_add_plugin(
   activitymanager_plugin_slc
   ${slc_SRCS}
   )

target_link_libraries(
   activitymanager_plugin_slc
   ${KDE4_KDECORE_LIBS}
   ${KDE4_KDEUI_LIBS}
   )

install(TARGETS activitymanager_plugin_slc DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES activitymanager-plugin-slc.desktop DESTINATION ${SERVICES_INSTALL_DIR})

