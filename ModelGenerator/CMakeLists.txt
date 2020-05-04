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
    model_export.cpp
    model_export.h
    image_utils.cpp
    image_utils.h
    general_utils.h
)

target_compile_definitions(ModelGenerator PRIVATE
    BOOST_UBLAS_CHECK_ENABLE
)

target_link_libraries(ModelGenerator PRIVATE
    Qt5::Widgets
    Boost::boost
    Boost::dynamic_linking
    Boost::serialization
)
