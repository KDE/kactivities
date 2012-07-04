PROJECT(KACTIVITIES)

FIND_PACKAGE(KDE4 REQUIRED)

INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)
INCLUDE(MacroOptionalAddSubdirectory)
INCLUDE(FindPackageHandleStandardArgs)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

add_subdirectory(src)

macro_display_feature_log ()
