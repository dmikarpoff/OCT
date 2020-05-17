cmake_minimum_required(VERSION 3.12)

project(KalmanFilter)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core REQUIRED)
find_library(OCT_UTILS_LIBRARY
    NAMES oct_utils
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../install_root/${CMAKE_BUILD_TYPE}/lib
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kalman_filter
    main.cpp
    kalman_filter.h
    kalman_filter.cpp
)

target_include_directories(kalman_filter
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../install_root/include
)

target_link_libraries(kalman_filter PRIVATE
    Qt5::Core
    ${OCT_UTILS_LIBRARY}
)

add_custom_command(
    TARGET kalman_filter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core> $<TARGET_FILE_DIR:kalman_filter>
)
add_custom_command(
    TARGET kalman_filter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/../install_root/${CMAKE_BUILD_TYPE}/bin/oct_utils.dll
        $<TARGET_FILE_DIR:kalman_filter>
)
