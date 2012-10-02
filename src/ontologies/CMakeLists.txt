configure_file (kao.ontology.in ${CMAKE_CURRENT_BINARY_DIR}/kao.ontology)

install (
    FILES
    kao.trig
    ${CMAKE_CURRENT_BINARY_DIR}/kao.ontology
    DESTINATION
    ${KACTIVITIES_ONTOLOGIES_DIR}
    )
