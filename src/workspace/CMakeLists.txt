macro_optional_find_package(Nepomuk)
macro_log_feature(
        Nepomuk_FOUND
        "Nepomuk" "Nepomuk" "http://www.kde.org" FALSE ""
        "STRONGLY_RECOMMENDED: Nepomuk is needed for the activities KIO slave and linking files to activities")

set(sdo_SRCS "")

if(Nepomuk_FOUND)
    add_subdirectory(fileitemplugin)
    add_subdirectory(kio)
endif(Nepomuk_FOUND)
