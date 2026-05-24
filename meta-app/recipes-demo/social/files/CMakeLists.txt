cmake_minimum_required(VERSION 3.16)
project(social LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick WebEngineQuick WebChannel Positioning)

add_executable(social
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qml.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/images/images.qrc
)

target_link_libraries(social
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::WebEngineQuick
        Qt6::WebChannel
        Qt6::Positioning
)

install(TARGETS social
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)