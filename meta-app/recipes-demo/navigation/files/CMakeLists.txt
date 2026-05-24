cmake_minimum_required(VERSION 3.16)
project(navigation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Positioning Location PositioningQuick VirtualKeyboard)

add_executable(navigation
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/images/images.qrc
)

target_link_libraries(navigation
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Positioning
        Qt6::Location
        Qt6::PositioningQuick
        Qt6::VirtualKeyboard
)

install(TARGETS navigation
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)