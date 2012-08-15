PROJECT(KACTIVITIES)

FIND_PACKAGE(KDE4 REQUIRED)

INCLUDE(KDE4Defaults)
INCLUDE(MacroLibrary)
INCLUDE(MacroOptionalAddSubdirectory)
INCLUDE(FindPackageHandleStandardArgs)

# Checking for Nepomuk
MACRO_OPTIONAL_FIND_PACKAGE(NepomukCore)
MACRO_LOG_FEATURE(NepomukCore_FOUND
    "Nepomuk Core" "Nepomuk Core" "https://projects.kde.org/nepomuk-core" FALSE ""
    "STRONGLY_RECOMMENDED: Nepomuk is needed for some activity-related info"
    )

    IF(NepomukCore_FOUND)
        FIND_PACKAGE(Soprano)
        MACRO_LOG_FEATURE(Soprano_FOUND
          "Soprano" "Semantic Desktop Storing" "http://soprano.sourceforge.net" TRUE ""
          "Soprano is needed to build and use the Nepomuk related functionalities"
          )

        include(SopranoAddOntology)
    ENDIF()

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

ADD_SUBDIRECTORY(src)

MACRO_DISPLAY_FEATURE_LOG ()
