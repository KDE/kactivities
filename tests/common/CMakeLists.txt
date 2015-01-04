find_package(KF5Activities 5.6 REQUIRED)

add_executable(statelogger state.cpp)
target_link_libraries(statelogger
    Qt5::Core
    KF5::Activities)
