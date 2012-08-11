macro_optional_find_package(NepomukCore)
macro_log_feature (NepomukCore_FOUND
    "Nepomuk Core" "Nepomuk Core" "https://projects.kde.org/projects/kde/kdelibs/nepomuk-core" FALSE ""
    "STRONGLY_RECOMMENDED: Nepomuk is needed for the activities KIO slave and linking files to activities")

set(sdo_SRCS "")

if(NepomukCore_FOUND)
    add_subdirectory(fileitemplugin)
    add_subdirectory(kio)
endif()
