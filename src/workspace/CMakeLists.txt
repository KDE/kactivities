
set (sdo_SRCS "")

# C++11

string (COMPARE EQUAL "${CMAKE_CXX_COMPILER_ID}" "Clang" CMAKE_COMPILER_IS_CLANG)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
    message (STATUS "We have GNU or Clang, adding -std=c++0x flag")
    add_definitions ("-std=c++0x")
    set (ADDITIONAL_DEFINITIONS "-std=c++0x")
endif ()

if (NepomukCore_FOUND)
    include_directories (${NEPOMUK_CORE_INCLUDE_DIR})
    add_subdirectory (fileitemplugin)
    add_subdirectory (kio)
endif ()

find_package (KDeclarative)

macro_log_feature (
    KDeclarative_FOUND "libkdeclarative" "KDE Declarative (QML) support from kdelibs is needed for the settings module" "http://www.kde.org"
    TRUE "" ""
)

if (KDeclarative_FOUND)
    add_subdirectory (settings)
endif ()
