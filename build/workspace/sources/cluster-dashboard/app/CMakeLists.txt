###########################################################################
# Copyright 2018,2019,2022,2025 Konsulko Group
#
# Author: Scott Murray <scott.murray@konsulko.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
###########################################################################

project(cluster-dashboard VERSION 2.0.0 LANGUAGES CXX)
include(GNUInstallDirs)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()
set(OE_QMAKE_PATH_EXTERNAL_HOST_BINS $ENV{OE_QMAKE_PATH_HOST_BINS})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core DBus Gui Network Quick Quick3D QuickControls2 QuickWidgets Positioning WebEngineQuick REQUIRED)
find_package(Qt6Gui ${QT_MIN_VERSION} CONFIG REQUIRED Private)
find_package(PkgConfig REQUIRED)
find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)

qt6_add_resources(RESOURCES cluster-dashboard.qrc images/images.qrc widgets/widgets.qrc myCluster/resource.qrc)

add_custom_command(
	OUTPUT  agl-shell-client-protocol.h
	COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header
	< ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell.xml
	> ${CMAKE_SOURCE_DIR}/app/agl-shell-client-protocol.h
	DEPENDS ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell.xml
)

add_custom_command(
	OUTPUT  ${CMAKE_BINARY_DIR}/app/agl-shell-client-protocol.h
	COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header
	< ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell.xml
	> ${CMAKE_SOURCE_DIR}/app/agl-shell-client-protocol.h
	DEPENDS ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell.xml
)

add_custom_command(
	OUTPUT  agl-shell-protocol.c
	COMMAND ${WAYLAND_SCANNER_EXECUTABLE} code
	< ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell.xml
	> ${CMAKE_BINARY_DIR}/app/agl-shell-protocol.c
	DEPENDS ${CMAKE_SOURCE_DIR}/app/protocol/agl-shell.xml
)

add_executable(${PROJECT_NAME}
	main.cpp
	myCluster/cpp_file/cluster.cpp
	myCluster/cpp_file/fpslogger.cpp
	myCluster/cpp_file/fuelarc.cpp
	myCluster/cpp_file/goongrestclient.cpp
	myCluster/cpp_file/startupnotifier.cpp
	myCluster/cpp_file/ShimmerLogoItem.cpp
	myCluster/cpp_file/PhoneNavigationBridge.cpp
	myCluster/header_file/PhoneNavigationBridge.h
	myCluster/header_file/ShimmerLogoItem.h
	myCluster/header_file/cluster.h
	myCluster/header_file/fpslogger.h
	myCluster/header_file/fuelarc.h
	myCluster/header_file/goongrestclient.h
	myCluster/header_file/startupnotifier.h
	agl-shell-protocol.c
	agl-shell-client-protocol.h
	${RESOURCES}
)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(QTAPPFW_VEHICLE_SIGNALS REQUIRED qtappfw-vehicle-signals)

include_directories(
	${Qt6Gui_PRIVATE_INCLUDE_DIRS}
	myCluster
	myCluster/header_file
	${QTAPPFW_VEHICLE_SIGNALS_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
	Qt6::Quick
	Qt6::DBus
	Qt6::Network
	Qt6::Quick3D
	Qt6::QuickControls2
	Qt6::QuickWidgets
	Qt6::Positioning
	Qt6::WebEngineQuick
	${QTAPPFW_VEHICLE_SIGNALS_LIBRARIES}
	${GLIB_LDFLAGS}
	${WAYLAND_CLIENT_LIBRARIES}
)

target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		MYCLUSTER_BUILD_3D=1
		MYCLUSTER_INSTALL_ASSET_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}/asset"
		MYCLUSTER_SOURCE_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/myCluster/asset"
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

install(DIRECTORY
	myCluster/asset/maps
	myCluster/asset/meshes
	myCluster/asset/textures
	DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/asset
)

install(FILES
	myCluster/EveningSkyHDRI018B_4K-HDR.exr
	myCluster/asset/qwantani_mid_morning_puresky_1k.hdr
	DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/asset
)
