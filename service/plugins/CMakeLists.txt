macro_optional_add_subdirectory(dummy)
macro_optional_add_subdirectory(slc)

macro_optional_find_package(Nepomuk)
if(Nepomuk_FOUND)
    macro_optional_add_subdirectory(nepomuk)
endif(Nepomuk_FOUND)
