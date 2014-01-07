project (KACTIVITIES)

find_package (KDE4 REQUIRED)

include (GenerateExportHeader)
include (KDE4Defaults)
include (MacroLibrary)
include (MacroOptionalAddSubdirectory)
include (FindPackageHandleStandardArgs)

# Until this is set by FindQt, we are defining the
# QT_NO_DEBUG_OUTPUT if this is not a debug build

string (TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
if (NOT CMAKE_BUILD_TYPE_TOLOWER MATCHES "debug")
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()


set (
    CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${CMAKE_MODULE_PATH}
    )

if (KAMD_PEDANTIC_COMPILATION)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif ()

add_subdirectory (src)

macro_display_feature_log ()
