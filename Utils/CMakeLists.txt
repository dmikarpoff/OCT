cmake_minimum_required(VERSION 3.12)

project(oct_utils)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

set(public_headers
    include/oct_utils/scan_data.h
    include/oct_utils/matrix_view.h
    include/oct_utils/export.h
)

add_library(oct_utils SHARED
    src/scan_data.cpp
    src/scan_data_loader.h
    ${public_headers}
)

target_compile_definitions(oct_utils PRIVATE
    OCT_UTILS_EXPORTS
)

target_include_directories(oct_utils PRIVATE
    include
)

target_link_libraries(oct_utils PUBLIC
    Qt5::Core
)

install(DIRECTORY include/oct_utils DESTINATION include)

install(TARGETS oct_utils
        CONFIGURATIONS Debug
        RUNTIME DESTINATION Debug/bin
        LIBRARY DESTINATION Debug/lib
        ARCHIVE DESTINATION Debug/lib
)
install(TARGETS oct_utils
        CONFIGURATIONS Release
        RUNTIME DESTINATION Release/bin
        LIBRARY DESTINATION Release/lib
        ARCHIVE DESTINATION Debug/lib
)
