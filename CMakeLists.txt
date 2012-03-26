PROJECT(KACTIVITIES)

FIND_PACKAGE(KDE4 REQUIRED)
INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)
INCLUDE(MacroOptionalAddSubdirectory)
INCLUDE(FindPackageHandleStandardArgs)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

# Installation directory for ontologies.  Needed here because used in both ontology/ and lib/ sub
# directories.
set(KACTIVITIES_ONTOLOGIES_DIR ${CMAKE_INSTALL_PREFIX}/share/ontology/kde)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

add_subdirectory(lib)
add_subdirectory(service)
add_subdirectory(ontologies)
