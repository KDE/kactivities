# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

set (sdo_SRCS "")

# C++11

add_subdirectory (fileitemplugin)
add_subdirectory (kio)

find_package(KF5Declarative)

macro_log_feature (
   KDeclarative_FOUND "libkdeclarative" "KDE Declarative (QML) support from kdelibs is needed for the settings module" "http://www.kde.org"
   TRUE "" ""
   )

if (KDeclarative_FOUND)
   add_subdirectory (settings)
endif ()
