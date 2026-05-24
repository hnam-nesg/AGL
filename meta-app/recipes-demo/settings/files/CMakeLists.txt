cmake_minimum_required(VERSION 3.16)
project(settings LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)
find_package(PkgConfig REQUIRED)
pkg_check_modules(QTAPPFW_BLUETOOTH REQUIRED qtappfw-bt)
pkg_check_modules(QTAPPFW_NETWORK REQUIRED qtappfw-network)
pkg_check_modules(QTAPPFW_VEHICLE_SIGNALS REQUIRED qtappfw-vehicle-signals)

add_executable(settings
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/images/images.qrc
)

target_include_directories(settings PRIVATE
    ${QTAPPFW_BLUETOOTH_INCLUDE_DIRS}
    ${QTAPPFW_NETWORK_INCLUDE_DIRS}
    ${QTAPPFW_VEHICLE_SIGNALS_INCLUDE_DIRS}
)

target_compile_options(settings PRIVATE
    ${QTAPPFW_BLUETOOTH_CFLAGS_OTHER}
    ${QTAPPFW_NETWORK_CFLAGS_OTHER}
    ${QTAPPFW_VEHICLE_SIGNALS_CFLAGS_OTHER}
)

target_link_directories(settings PRIVATE
    ${QTAPPFW_BLUETOOTH_LIBRARY_DIRS}
    ${QTAPPFW_NETWORK_LIBRARY_DIRS}
    ${QTAPPFW_VEHICLE_SIGNALS_LIBRARY_DIRS}
)


target_link_libraries(settings
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        ${QTAPPFW_BLUETOOTH_LIBRARIES}
        ${QTAPPFW_NETWORK_LIBRARIES}
        ${QTAPPFW_VEHICLE_SIGNALS_LIBRARIES}
)

install(TARGETS settings
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)