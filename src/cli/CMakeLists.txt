# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesCLI)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)
find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)

add_executable(kactivities-cli6)
ecm_mark_nongui_executable(kactivities-cli6)

target_include_directories(kactivities-cli6 PRIVATE
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
)

target_sources(kactivities-cli6 PRIVATE
   main.cpp
)

target_link_libraries(kactivities-cli6
   Qt6::Core
   KActivities
)

install (TARGETS
   kactivities-cli6
   ${CMAKECONFIG_INSTALL_DIR}
)
