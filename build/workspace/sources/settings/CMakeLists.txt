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
pkg_check_modules(BLUEZ_GLIB REQUIRED bluez-glib)
pkg_check_modules(GLIB REQUIRED glib-2.0 gio-2.0 gobject-2.0 gio-unix-2.0)
pkg_check_modules(QTAPPFW_NETWORK REQUIRED qtappfw-network)
pkg_check_modules(QTAPPFW_VEHICLE_SIGNALS REQUIRED qtappfw-vehicle-signals)

add_executable(settings
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bluetoothdevicemodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bluetoothdevicemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bluetoothmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bluetoothmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bluetoothworker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bluetoothworker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assistantsettingsmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assistantsettingsmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/images/images.qrc
)

target_include_directories(settings PRIVATE
    ${BLUEZ_GLIB_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${QTAPPFW_NETWORK_INCLUDE_DIRS}
    ${QTAPPFW_VEHICLE_SIGNALS_INCLUDE_DIRS}
)

target_compile_options(settings PRIVATE
    ${BLUEZ_GLIB_CFLAGS_OTHER}
    ${GLIB_CFLAGS_OTHER}
    ${QTAPPFW_NETWORK_CFLAGS_OTHER}
    ${QTAPPFW_VEHICLE_SIGNALS_CFLAGS_OTHER}
)

target_link_directories(settings PRIVATE
    ${BLUEZ_GLIB_LIBRARY_DIRS}
    ${GLIB_LIBRARY_DIRS}
    ${QTAPPFW_NETWORK_LIBRARY_DIRS}
    ${QTAPPFW_VEHICLE_SIGNALS_LIBRARY_DIRS}
)


target_link_libraries(settings
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        ${BLUEZ_GLIB_LIBRARIES}
        ${GLIB_LIBRARIES}
        ${QTAPPFW_NETWORK_LIBRARIES}
        ${QTAPPFW_VEHICLE_SIGNALS_LIBRARIES}
)

install(TARGETS settings
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
