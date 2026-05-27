cmake_minimum_required(VERSION 3.16)
project(social LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick WebEngineQuick WebChannel Positioning DBus)

set(COMMON_THEME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../common/theme")
if(NOT EXISTS "${COMMON_THEME_DIR}/ThemeSettingsManager.h")
    set(COMMON_THEME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../common/theme")
endif()

add_executable(social
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InputInjector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/InputInjector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SocialDbus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SocialDbus.h
    ${COMMON_THEME_DIR}/ThemeSettingsManager.h
    ${COMMON_THEME_DIR}/ThemeSettingsManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qml.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/images/images.qrc
)

target_include_directories(social PRIVATE
    ${COMMON_THEME_DIR}
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
        Qt6::DBus
)

install(TARGETS social
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
