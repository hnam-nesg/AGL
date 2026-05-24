cmake_minimum_required(VERSION 3.16)
project(navigation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Network DBus Positioning Location PositioningQuick WebEngineQuick WebChannel)
find_package(SQLite3 REQUIRED)

add_executable(navigation
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mbtilesserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mbtilesserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/goongrestclient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/goongrestclient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/navigationdbusservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/navigationdbusservice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/searchcontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/searchcontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/images/images.qrc
)

target_link_libraries(navigation
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Network
        Qt6::DBus
        Qt6::Positioning
        Qt6::Location
        Qt6::PositioningQuick
        Qt6::WebEngineQuick
        Qt6::WebChannel
        SQLite::SQLite3
)

install(TARGETS navigation
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/dbus/org.agl.navigation.conf
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d
)
