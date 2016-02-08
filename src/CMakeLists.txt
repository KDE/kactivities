# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

# Boosting us a bit

if (NOT KACTIVITIES_LIBRARY_ONLY)
   find_package (Boost 1.49 REQUIRED)

   string (REGEX MATCH "1053.." BOOST_VERSION_BLACKLISTED ${Boost_VERSION})

   if (BOOST_VERSION_BLACKLISTED AND NOT KACTIVITIES_ENABLE_EXCEPTIONS)
      message (
         WARNING
         "Boost.Container 1.53 has issues when exceptions are disabled. "
         "We will set the KACTIVITIES_ENABLE_EXCEPTIONS option."
         )
      set (KACTIVITIES_ENABLE_EXCEPTIONS ON)
   endif ()
endif ()

if (KACTIVITIES_ENABLE_EXCEPTIONS)
   string (REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
   add_definitions (-fexceptions)
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
   message (
      FATAL_ERROR
      "You need a C++ compiler that supports the following C++11 features: auto, lambdas and move semantics."
      )
endif ()

# =======================================================
# Starting the actual project definition

# The libraries do not depend on any compile-time features
add_subdirectory (lib)

if (NOT KACTIVITIES_LIBRARY_ONLY)
   include_directories (
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}
      )
   add_subdirectory (imports)
endif ()

