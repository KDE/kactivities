# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

option (KACTIVITIES_LIBRARY_ONLY "If true, compiles only the KActivities library, without the service and other modules." OFF)
option (KACTIVITIES_ENABLE_EXCEPTIONS "If you have Boost 1.53, you need to build KActivities with exceptions enabled. This is UNTESTED and EXPERIMENTAL!" OFF)

# Boosting us a bit

if (NOT KACTIVITIES_LIBRARY_ONLY)
   find_package (Boost 1.49 REQUIRED)

   string (REGEX MATCH "1053.." BOOST_VERSION_BLACKLISTED ${Boost_VERSION})

   if (BOOST_VERSION_BLACKLISTED AND NOT KACTIVITIES_ENABLE_EXCEPTIONS)
      message(
         WARNING
         "Boost.Container 1.53 has issues when exceptions are disabled. "
         "We will set the KACTIVITIES_ENABLE_EXCEPTIONS option."
      )
      set(KACTIVITIES_ENABLE_EXCEPTIONS ON)
   endif()
endif ()

if (KACTIVITIES_ENABLE_EXCEPTIONS)
   string(REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
   add_definitions(-fexceptions)
endif()

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
set (KAMD_DATA_DIR "${KF5_DATA_INSTALL_DIR}/kactivitymanagerd/")
set (KAMD_PLUGIN_DIR "${PLUGIN_INSTALL_DIR}/kactivitymanagerd/")
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
      message (WARNING
         "WARNING: You need boost libraries (range and containers) in order to build the Activity Manager daemon. We will only build the library, setting the KACTIVITIES_LIBRARY_ONLY option to ON."
         )
      set(KACTIVITIES_LIBRARY_ONLY ON)
   endif ()

   if (NOT HAVE_CXX11_VARIADIC_TEMPLATES OR NOT HAVE_CXX11_INITIALIZER_LISTS)
      message (WARNING
         "WARNING: You need a more modern compiler in order to build the Activity Manager daemon. We will only build the library, setting the KACTIVITIES_LIBRARY_ONLY option to ON."
         )
      set(KACTIVITIES_LIBRARY_ONLY ON)
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

   add_subdirectory (imports)
   add_subdirectory (service)

   # No point in having workspace addons without the service
   # add_subdirectory (workspace)

endif ()

