cmake_minimum_required(VERSION 3.16)
project(assistant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Concurrent Multimedia)

add_executable(assistant
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/asrmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/asrmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/asrworker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/asrworker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/whisperengine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/whisperengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc
)

target_link_libraries(assistant
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Concurrent
        Qt6::Multimedia
        whisper
)

install(TARGETS assistant
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)