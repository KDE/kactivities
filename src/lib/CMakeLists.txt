# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

add_subdirectory (core)

if (KACTIVITIES_INSTALL_EXPERIMENTAL_LIBRARIES)
   add_subdirectory (stats)
endif()

