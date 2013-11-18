# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

# Installation directory for ontologies. Needed here because
# it is used in both ontology/ and lib/ subdirectories.
set (KACTIVITIES_ONTOLOGIES_DIR ${CMAKE_INSTALL_PREFIX}/share/ontology/kde)

option (KACTIVITIES_LIBRARY_ONLY "If true, compiles only the kactivities library, without the service and other modules." OFF)

# TODO: Add support for Nepomuk when it comes to Qt5 world
# # Checking for Nepomuk
# macro_optional_find_package (NepomukCore)
# macro_log_feature (
#     NepomukCore_FOUND
#     "Nepomuk Core" "Nepomuk Core" "https://projects.kde.org/nepomuk-core" FALSE ""
#     "STRONGLY_RECOMMENDED: Nepomuk is needed for some activity-related info"
#     )
#
# if (NepomukCore_FOUND)
#     find_package (Soprano)
#     macro_log_feature (
#         Soprano_FOUND
#         "Soprano" "Semantic Desktop Storing" "http://soprano.sourceforge.net" TRUE ""
#         "Soprano is needed to build and use the Nepomuk related functionalities"
#         )
#
#     include (SopranoAddOntology)
# endif ()
#
# if (NepomukCore_FOUND AND Soprano_FOUND)
#     set (HAVE_NEPOMUK 1)
# endif ()

# Boosting us a bit

if (NOT KACTIVITIES_LIBRARY_ONLY)
   find_package (Boost 1.54 REQUIRED)
endif ()

# Testing for C++0x/C++11 features
include (CheckCxxFeatures)
cxx_check_feature ("c++11" "auto"               "N2546" HAVE_CXX11_AUTO               "${ADDITIONAL_DEFINITIONS}")
cxx_check_feature ("c++11" "nullptr"            "N2431" HAVE_CXX11_NULLPTR            "${ADDITIONAL_DEFINITIONS}")
cxx_check_feature ("c++11" "lambda"             "N2927" HAVE_CXX11_LAMBDA             "${ADDITIONAL_DEFINITIONS}")
cxx_check_feature ("c++11" "override"           "N3206" HAVE_CXX11_OVERRIDE           "${ADDITIONAL_DEFINITIONS}")
cxx_check_feature ("c++11" "unique_ptr"         "none"  HAVE_CXX11_UNIQUE_PTR         "${ADDITIONAL_DEFINITIONS}")
cxx_check_feature ("c++11" "variadic-templates" "N2242" HAVE_CXX11_VARIADIC_TEMPLATES "${ADDITIONAL_DEFINITIONS}")
cxx_check_feature ("c++11" "initializer-lists"  "N2672" HAVE_CXX11_INITIALIZER_LISTS  "${ADDITIONAL_DEFINITIONS}")

# We can now actually require some C++11 features even for the library
if (NOT HAVE_CXX11_AUTO OR NOT HAVE_CXX11_LAMBDA OR NOT HAVE_CXX11_UNIQUE_PTR)
   message(
      FATAL_ERROR
      "You need a C++ compiler that supports the following C++11 features: auto, lambdas and move semantics."
   )
endif()

# =======================================================
# Starting the actual project definition

# The libraries do not depend on any compile-time features
add_subdirectory (lib)

# Config file
set (KAMD_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set (KAMD_DATA_DIR "${DATA_INSTALL_DIR}/activitymanager/")
set (KAMD_PLUGIN_DIR "${PLUGIN_INSTALL_DIR}/activitymanager/")
configure_file (kactivities-features.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kactivities-features.h)

include_directories(
   ${CMAKE_CURRENT_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}/lib/core
   )

# Is the compiler modern enough to build the ActivityManager service
# and accompanying workspace addons?

string (COMPARE EQUAL "${CXX_FEATURES_UNSUPPORTED}" "" CXX_COMPILER_IS_MODERN)

if (NOT KACTIVITIES_LIBRARY_ONLY)

   if (NOT Boost_FOUND)
      message (FATAL_ERROR
         "REQUIRED: You need boost libraries (range and containers) in order to build the Activity Manager daemon. If you want to build only the library, set the KACTIVITIES_LIBRARY_ONLY option to ON."
         )
   endif ()

   if (NOT HAVE_CXX11_VARIADIC_TEMPLATES OR NOT HAVE_CXX11_INITIALIZER_LISTS)
      message (FATAL_ERROR
         "REQUIRED: You need a more modern compiler in order to build the Activity Manager daemon. If you want to build only the library, set the KACTIVITIES_LIBRARY_ONLY option to ON."
         )
   endif ()

   include_directories (${Boost_INCLUDE_DIR})

   # The compiler is good enough
   if (CXX_COMPILER_IS_MODERN)
      message (STATUS
         "C++11 enabled compiler: Your compiler is state-of-the-art"
         )
   else ()
      message (STATUS
         "C++11 enabled compiler:"
         "Your compiler doesn't support the following features: ${CXX_FEATURES_UNSUPPORTED} but
         the list of the supported ones is sufficient for the build: ${CXX_FEATURES_SUPPORTED}"
         )
   endif ()

   add_subdirectory (service)

   # No point in having workspace addons without the service
   # add_subdirectory (workspace)

endif ()

add_subdirectory (ontologies)

