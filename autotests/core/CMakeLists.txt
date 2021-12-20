# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesTest)

find_package (Qt${QT_MAJOR_VERSION} REQUIRED NO_MODULE COMPONENTS Test Core DBus)

include_directories (
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/tests/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
   ${CMAKE_BINARY_DIR}/src/
   )

set (
   KActivitiesTest_SRCS
   main.cpp
   Process.cpp
   OfflineTest.cpp
   CleanOnlineTest.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/common/test.cpp
   )

if (NOT WIN32)

   add_executable (
      KActivitiesTest
      ${KActivitiesTest_SRCS}
      )

   target_link_libraries (
      KActivitiesTest
      Qt${QT_MAJOR_VERSION}::Core
      Qt${QT_MAJOR_VERSION}::Test
      Qt${QT_MAJOR_VERSION}::DBus
      KF5::Activities
      )

endif ()
