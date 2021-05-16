# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesCLI)

find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)
find_package (Qt5 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)
find_package (KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS WindowSystem)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
   )

set (
   KActivitiesCLI_SRCS
   main.cpp
   )

qt_wrap_ui(
   KActivitiesCLI_SRCS
   )

add_executable (
   kactivities-cli
   ${KActivitiesCLI_SRCS}
   )

target_link_libraries (
   kactivities-cli
   Qt5::Core
   KF5::Activities
   )

ecm_mark_nongui_executable(
   kactivities-cli
   )

install (TARGETS
   kactivities-cli
   ${KF5_INSTALL_TARGETS_DEFAULT_ARGS}
   )
