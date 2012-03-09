configure_file(kao.ontology.in ${CMAKE_CURRENT_BINARY_DIR}/kao.ontology)

install (
  FILES
    kao.trig
    ${CMAKE_CURRENT_BINARY_DIR}/kao.ontology
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/share/ontology/kde
)
