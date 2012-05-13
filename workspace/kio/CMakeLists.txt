project(kioslave-activities)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${KDE4_KIO_INCLUDES}
    ${NEPOMUK_INCLUDE_DIR}
    )

########### next target ###############

set(kio_activities_PART_SRCS
    kio_activities.cpp
    )

soprano_add_ontology(
    kio_activities_PART_SRCS
    ${CMAKE_SOURCE_DIR}/ontologies/kao.trig
    "KAO" "Nepomuk::Vocabulary" "trig"
    )


kde4_add_plugin(
    kio_activities
    ${kio_activities_PART_SRCS}
    )

target_link_libraries(
    kio_activities
    ${KDE4_KIO_LIBS}
    ${NEPOMUK_LIBRARIES}
    ${NEPOMUK_QUERY_LIBRARIES}
    ${SOPRANO_LIBRARIES}
    kactivities
    )

install(
    TARGETS kio_activities
    DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES activities.protocol  DESTINATION  ${SERVICES_INSTALL_DIR})
