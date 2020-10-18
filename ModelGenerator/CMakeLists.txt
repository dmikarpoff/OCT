cmake_minimum_required(VERSION 3.5)

project(ModelGenerator LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOOST_ROOT ${CMAKE_SOURCE_DIR}/../boost_1_72_0/boost)
set(BOOST_INCLUDEDIR ${CMAKE_SOURCE_DIR}/../boost_1_72_0)
set(BOOST_LIBRARYDIR ${CMAKE_SOURCE_DIR}/../boost_1_72_0/lib64-msvc-14.0)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Boost REQUIRED system serialization)
find_library(OCT_UTILS_LIBRARY
    NAMES oct_utils
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/../install_root/${CMAKE_BUILD_TYPE}/lib
)

add_executable(ModelGenerator
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    layer_property_widget.cpp
    layer_property_widget.h
    layer_property_widget.ui
    layer_property.h
    tissuesolver.cpp
    tissuesolver.h
    iterativetissuesolver.cpp
    iterativetissuesolver.h
    colorselectionwidget.cpp
    colorselectionwidget.h
    colorselectionwidget.ui
    generationprogressdialog.cpp
    generationprogressdialog.h
    generationprogressdialog.ui
    general_utils.h
)

target_compile_definitions(ModelGenerator PRIVATE
    BOOST_UBLAS_CHECK_ENABLE
    NOMINMAX
)

target_include_directories(ModelGenerator
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../install_root/include
)

target_link_libraries(ModelGenerator PRIVATE
    Qt5::Widgets
    Boost::boost
    Boost::dynamic_linking
    Boost::serialization
    ${OCT_UTILS_LIBRARY}
)

add_custom_command(
    TARGET ModelGenerator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::Core>
                                                  $<TARGET_FILE:Qt5::Gui>
                                                  $<TARGET_FILE:Qt5::Widgets>
                                    $<TARGET_FILE_DIR:ModelGenerator>
)

add_custom_command(
    TARGET ModelGenerator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ModelGenerator>/platforms
)

add_custom_command(
    TARGET ModelGenerator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt5::QWindowsIntegrationPlugin>
                                $<TARGET_FILE_DIR:ModelGenerator>/platforms
)
add_custom_command(
    TARGET ModelGenerator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/../install_root/${CMAKE_BUILD_TYPE}/bin/oct_utils.dll
        $<TARGET_FILE_DIR:ModelGenerator>
)

