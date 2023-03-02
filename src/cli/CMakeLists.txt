# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesCLI)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)
find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)
find_package (KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS WindowSystem)

add_executable(kactivities-cli)
ecm_mark_nongui_executable(kactivities-cli)

target_include_directories(kactivities-cli PRIVATE
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
)

target_sources(kactivities-cli PRIVATE
   main.cpp
)

target_link_libraries(kactivities-cli
   Qt6::Core
   KF6::Activities
)

install (TARGETS
   kactivities-cli
   ${KF_INSTALL_TARGETS_DEFAULT_ARGS}
)
