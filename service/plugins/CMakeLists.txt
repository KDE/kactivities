macro_optional_add_subdirectory(slc)
macro_optional_add_subdirectory(sqlite)
macro_optional_add_subdirectory(globalshortcuts)

get_property(KACTIVITIES_BUILD_NEPOMUK_PLUGIN GLOBAL PROPERTY KACTIVITIES_BUILD_NEPOMUK_PLUGIN)
get_property(KACTIVITIES_BUILD_DUMMY_PLUGIN GLOBAL PROPERTY KACTIVITIES_BUILD_DUMMY_PLUGIN)

if(KACTIVITIES_BUILD_ACTIVITYRANKING_PLUGIN)
    macro_optional_add_subdirectory(activityranking)
endif()

if(KACTIVITIES_BUILD_NEPOMUK_PLUGIN AND Nepomuk_FOUND)
    macro_optional_add_subdirectory(nepomuk)
endif()

if (KACTIVITIES_BUILD_DUMMY_PLUGIN)
    macro_optional_add_subdirectory(dummy)
endif()
