cmake_minimum_required(VERSION 3.10)
project(openWakeWordCpp VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# BỎ DÙNG PKG-CONFIG, DÙNG TÌM KIẾM TRỰC TIẾP TRONG SYSROOT CỦA YOCTO
find_path(ONNXRUNTIME_INCLUDE_DIR NAMES onnxruntime_cxx_api.h)
find_library(ONNXRUNTIME_LIBRARY NAMES onnxruntime)

# Kiểm tra an toàn
if(NOT ONNXRUNTIME_INCLUDE_DIR OR NOT ONNXRUNTIME_LIBRARY)
    message(FATAL_ERROR "KHÔNG THỂ TÌM THẤY ONNX Runtime! Vui lòng kiểm tra lại recipe onnxruntime.")
endif()

message(STATUS "Found ONNX Runtime Include: ${ONNXRUNTIME_INCLUDE_DIR}")
message(STATUS "Found ONNX Runtime Library: ${ONNXRUNTIME_LIBRARY}")

# Chỉ định thư mục chứa header
include_directories(include ${ONNXRUNTIME_INCLUDE_DIR})

# Tạo Shared Library
add_library(wakeword SHARED src/openwakeword_engine.cpp)

# Link thư viện (trỏ thẳng vào biến mà find_library vừa tìm được)
target_link_libraries(wakeword ${ONNXRUNTIME_LIBRARY})

# Cài đặt file
include(GNUInstallDirs)
install(TARGETS wakeword DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openwakeword)
