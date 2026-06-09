cmake_minimum_required(VERSION 3.21)
project(PetekCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt yolunu CMake çağrısında -DCMAKE_PREFIX_PATH ile vereceğiz
find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)

add_executable(PetekCpp
    main.cpp
    mainwindow.h
    mainwindow.cpp
    PetekA.ui
)

target_link_libraries(PetekCpp PRIVATE Qt6::Widgets Qt6::Svg)
