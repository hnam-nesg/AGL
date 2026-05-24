cmake_minimum_required(VERSION 3.16)
project(hvac LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick ShaderTools DBus)

find_package(PkgConfig REQUIRED)
pkg_check_modules(QTAPPFW_HVAC REQUIRED qtappfw-hvac)
pkg_check_modules(QTAPPFW_VEHICLE_SIGNALS REQUIRED qtappfw-vehicle-signals)

add_executable(hvac
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HvacUiDbus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HvacUiDbus.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hvac.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/images/images.qrc
)

target_include_directories(hvac PRIVATE
    ${QTAPPFW_HVAC_INCLUDE_DIRS}
    ${QTAPPFW_VEHICLE_SIGNALS_INCLUDE_DIRS}
)

target_compile_options(hvac PRIVATE
    ${QTAPPFW_HVAC_CFLAGS_OTHER}
    ${QTAPPFW_VEHICLE_SIGNALS_CFLAGS_OTHER}
)

target_link_directories(hvac PRIVATE
    ${QTAPPFW_HVAC_LIBRARY_DIRS}
    ${QTAPPFW_VEHICLE_SIGNALS_LIBRARY_DIRS}
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
        Qt6::DBus
        ${QTAPPFW_HVAC_LIBRARIES}
        ${QTAPPFW_VEHICLE_SIGNALS_LIBRARIES}
)

install(TARGETS hvac
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dbus/org.agl.hvac.conf
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d
)
