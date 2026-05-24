cmake_minimum_required(VERSION 3.16)
project(light LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Quick3D)

add_executable(light
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/images/images.qrc
)

target_link_libraries(light
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Quick3D
)

install(TARGETS light
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)