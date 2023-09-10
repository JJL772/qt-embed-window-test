
project(qt-window-test CXX)
cmake_minimum_required(VERSION 3.15)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(SDL2 REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(
    qt-window-test

    window-test.cpp
    window-test.h
)

target_link_libraries(
    qt-window-test PRIVATE

    SDL2 Qt6::Core Qt6::Widgets Qt6::Gui Qt6::CorePrivate Qt6::WidgetsPrivate Qt6::GuiPrivate GLEW GL
)
