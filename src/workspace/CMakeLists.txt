
set(sdo_SRCS "")

if(NepomukCore_FOUND)
    add_subdirectory(fileitemplugin)
    add_subdirectory(kio)
endif()
