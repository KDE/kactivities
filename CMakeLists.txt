project (KACTIVITIES)

find_package (KDE4 REQUIRED)

include (KDE4Defaults)
include (MacroLibrary)
include (MacroOptionalAddSubdirectory)
include (FindPackageHandleStandardArgs)

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
