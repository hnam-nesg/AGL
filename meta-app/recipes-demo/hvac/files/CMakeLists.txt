cmake_minimum_required(VERSION 3.16)
project(hvac LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick ShaderTools)

add_executable(hvac
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hvac.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/images/images.qrc
)

qt6_add_shaders(hvac "app_shaders"
    PREFIX "/"
    FILES
        windflow.frag
)

target_link_libraries(hvac
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
)

install(TARGETS hvac
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)