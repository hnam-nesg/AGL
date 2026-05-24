cmake_minimum_required(VERSION 3.16)
project(mediaplayer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

find_package(PkgConfig REQUIRED)
pkg_check_modules(QTAPPFW_MEDIAPLAYER REQUIRED qtappfw-mediaplayer)
pkg_check_modules(QTAPPFW_VEHICLE_SIGNALS REQUIRED qtappfw-vehicle-signals)

add_executable(mediaplayer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/images/images.qrc
)

target_include_directories(mediaplayer PRIVATE
    ${QTAPPFW_MEDIAPLAYER_INCLUDE_DIRS}
    ${QTAPPFW_VEHICLE_SIGNALS_INCLUDE_DIRS}
)

target_compile_options(mediaplayer PRIVATE
    ${QTAPPFW_MEDIAPLAYER_CFLAGS_OTHER}
    ${QTAPPFW_VEHICLE_SIGNALS_CFLAGS_OTHER}
)

target_link_directories(mediaplayer PRIVATE
    ${QTAPPFW_MEDIAPLAYER_LIBRARY_DIRS}
    ${QTAPPFW_VEHICLE_SIGNALS_LIBRARY_DIRS}
)


target_link_libraries(mediaplayer
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        ${QTAPPFW_MEDIAPLAYER_LIBRARIES}
        ${QTAPPFW_VEHICLE_SIGNALS_LIBRARIES}
)

install(TARGETS mediaplayer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)