
set(sdo_SRCS "")

if(NepomukCore_FOUND)
    include_directories(${NEPOMUK_CORE_INCLUDE_DIR})
    add_subdirectory(fileitemplugin)
    add_subdirectory(kio)
endif()
