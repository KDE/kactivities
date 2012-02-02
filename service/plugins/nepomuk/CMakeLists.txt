project(activitymanager-plugin-nepomuk)

# Checking for Nepomuk
find_package(Nepomuk REQUIRED)

include_directories(${NEPOMUK_INCLUDE_DIR} ${SOPRANO_INCLUDE_DIR})

set(
   nepomuk_SRCS
   NepomukCommon.cpp
   NepomukPlugin.cpp
   NepomukResourceScoreCache.cpp
   NepomukResourceScoreMaintainer.cpp
   Rankings.cpp
   ../../Plugin.cpp
   ../../SharedInfo.cpp
   )

qt4_add_dbus_adaptor(
   nepomuk_SRCS
   org.kde.ActivityManager.Rankings.xml
   Rankings.h Rankings
   )

soprano_add_ontology(nepomuk_SRCS
    ${CMAKE_SOURCE_DIR}/ontologies/kext.trig
    "KExt" "Nepomuk::Vocabulary" "trig"
    )

# soprano_add_ontology(nepomuk_SRCS
#     ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nepomuk/nao.trig
#     "NAO" "Nepomuk::Vocabulary" "trig"
#     )

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}/../..
   ${KDE4_INCLUDES}
   )

kde4_add_plugin(
   activitymanager_plugin_nepomuk
   ${nepomuk_SRCS}
   )

target_link_libraries(
   activitymanager_plugin_nepomuk
   ${KDE4_KDECORE_LIBS}
   ${KDE4_KDEUI_LIBS}
   ${NEPOMUK_LIBRARIES}
   ${NEPOMUK_QUERY_LIBRARIES}
   ${SOPRANO_LIBRARIES}
   )

install(TARGETS activitymanager_plugin_nepomuk DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES activitymanager-plugin-nepomuk.desktop DESTINATION ${SERVICES_INSTALL_DIR})

