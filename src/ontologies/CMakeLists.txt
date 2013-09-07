# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

configure_file (kao.ontology.in ${CMAKE_CURRENT_BINARY_DIR}/kao.ontology)

install (
   FILES
   kao.trig
   ${CMAKE_CURRENT_BINARY_DIR}/kao.ontology
   DESTINATION
   ${KACTIVITIES_ONTOLOGIES_DIR}
   )
