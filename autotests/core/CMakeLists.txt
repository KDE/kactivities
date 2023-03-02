# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesTest)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Test Core DBus)

if (NOT WIN32)

add_executable(KActivitiesTest)

target_include_directories(KActivitiesTest PRIVATE
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/tests/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
   ${CMAKE_BINARY_DIR}/src/
)

target_sources(KActivitiesTest PRIVATE
   main.cpp
   Process.cpp
   OfflineTest.cpp
   CleanOnlineTest.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/common/test.cpp
)

target_link_libraries(KActivitiesTest
   PRIVATE
      Qt6::Core
      Qt6::Test
      Qt6::DBus
      KF6::Activities
)

endif ()
