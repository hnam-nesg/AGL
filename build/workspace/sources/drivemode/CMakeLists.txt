cmake_minimum_required(VERSION 3.16)
project(drivemode LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

add_executable(drivemode
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/images/images.qrc
)

target_link_libraries(drivemode
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
)

install(TARGETS drivemode
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)