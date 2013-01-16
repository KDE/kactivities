macro_optional_add_subdirectory (slc)
macro_optional_add_subdirectory (activityranking)
macro_optional_add_subdirectory (sqlite)
macro_optional_add_subdirectory (globalshortcuts)
macro_optional_add_subdirectory (virtualdesktopswitch)

if (NepomukCore_FOUND AND Soprano_FOUND)
    macro_optional_add_subdirectory (nepomuk)
endif ()
