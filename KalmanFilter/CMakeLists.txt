cmake_minimum_required(VERSION 3.12)

project(KalmanFilter)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kalman_filter
    main.cpp
    scan_data_loader.h
    scan_data.h
    scan_data.cpp
)

target_link_libraries(kalman_filter PRIVATE
    Qt5::Core
)

add_custom_command(
    TARGET kalman_filter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:kalman_filter>
)
