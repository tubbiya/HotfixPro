cmake_minimum_required(VERSION 3.22)
project(PetekCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)  # .ui dosyasını otomatik derle

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Gui)

add_executable(PetekCpp
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    resources.qrc
)

target_link_libraries(PetekCpp PRIVATE Qt6::Widgets Qt6::Svg Qt6::Gui)
