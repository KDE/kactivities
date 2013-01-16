project (kioslave-activities)

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    ${SOPRANO_INCLUDE_DIR}
    )

########### next target ###############

set (kio_activities_PART_SRCS
    KioActivities.cpp
    )

soprano_add_ontology (
    kio_activities_PART_SRCS
    ${CMAKE_SOURCE_DIR}/src/ontologies/kao.trig
    "KAO" "KDE::Vocabulary" "trig"
    )

set_source_files_properties (
    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
    PROPERTIES
    INCLUDE ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.h
    )

qt4_add_dbus_interface (
    kio_activities_PART_SRCS
    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.xml
    activities_interface
    )

qt4_add_dbus_interface (
    kio_activities_PART_SRCS
    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Resources.xml
    resources_interface
    )

kde4_add_plugin (
    kio_activities
    ${kio_activities_PART_SRCS}
    )

target_link_libraries (
    kio_activities
    ${KDE4_KIO_LIBS}
    ${NEPOMUK_CORE_LIBRARY}
    ${SOPRANO_LIBRARIES}
    kactivities
    )

install (
    TARGETS
    kio_activities
    DESTINATION
    ${PLUGIN_INSTALL_DIR}
    )

########### install files ###############

install (
    FILES
    activities.protocol
    DESTINATION
    ${SERVICES_INSTALL_DIR}
    )
