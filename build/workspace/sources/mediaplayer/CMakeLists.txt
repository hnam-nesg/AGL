cmake_minimum_required(VERSION 3.16)
project(mediaplayer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core DBus Gui Qml Quick)

add_executable(mediaplayer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audiocontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audiocontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bluetoothmediaplayer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bluetoothmediaplayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mediadbusservice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mediadbusservice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/qml.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/images/images.qrc
)

target_link_libraries(mediaplayer
    PRIVATE
        Qt6::Core
        Qt6::DBus
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
)

install(TARGETS mediaplayer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.agl.media.conf
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/system.d
)
