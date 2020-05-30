cmake_minimum_required(VERSION 3.12)

project(KalmanFilter)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(BOOST_ROOT ${CMAKE_SOURCE_DIR}/../boost_1_72_0/boost)
set(BOOST_INCLUDEDIR ${CMAKE_SOURCE_DIR}/../boost_1_72_0)
set(BOOST_LIBRARYDIR ${CMAKE_SOURCE_DIR}/../boost_1_72_0/lib64-msvc-14.0)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Boost REQUIRED)
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
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_include_directories(kalman_filter
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../install_root/include
)

target_link_libraries(kalman_filter PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Boost::boost
    ${OCT_UTILS_LIBRARY}
)

target_compile_definitions(kalman_filter PRIVATE _USE_MATH_DEFINES)

add_custom_command(
    TARGET kalman_filter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core>
                                                  $<TARGET_FILE:Qt5::Gui>
                                                  $<TARGET_FILE:Qt5::Widgets>
                                    $<TARGET_FILE_DIR:kalman_filter>
)

add_custom_command(
    TARGET kalman_filter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:kalman_filter>/platforms
)

add_custom_command(
    TARGET kalman_filter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>
                                $<TARGET_FILE_DIR:kalman_filter>/platforms
)
add_custom_command(
    TARGET kalman_filter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/../install_root/${CMAKE_BUILD_TYPE}/bin/oct_utils.dll
        $<TARGET_FILE_DIR:kalman_filter>
)
