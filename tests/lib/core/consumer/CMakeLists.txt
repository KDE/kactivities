# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesTest)

find_package(Qt5 REQUIRED NO_MODULE COMPONENTS Test Core DBus)

include_directories(
   ${CMAKE_SOURCE_DIR}/src/lib/core/
   ${CMAKE_BINARY_DIR}/src/lib/core/
   )

set (
   KActivitiesTest_SRCS
   main.cpp
   ConsumerTest.cpp
   )

add_executable (
   KActivitiesTest
   ${KActivitiesTest_SRCS}
   )

target_link_libraries (
   KActivitiesTest
   Qt5::Core
   Qt5::Test
   Qt5::DBus
   ${CMAKE_BINARY_DIR}/src/lib/core/libKActivities.so
   )

