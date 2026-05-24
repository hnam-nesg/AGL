cmake_minimum_required(VERSION 3.20)
project(server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(absl CONFIG REQUIRED)

# Nhận từ Yocto nếu recipe truyền vào.
# Nếu không có thì tự tìm theo ngữ cảnh build.
if(NOT DEFINED PROTOC_EXECUTABLE)
    if(CMAKE_CROSSCOMPILING)
        find_program(PROTOC_EXECUTABLE protoc REQUIRED)
    else()
        set(PROTOC_EXECUTABLE $<TARGET_FILE:protobuf::protoc>)
    endif()
endif()

if(NOT DEFINED GRPC_CPP_PLUGIN_EXECUTABLE)
    if(CMAKE_CROSSCOMPILING)
        find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin REQUIRED)
    elseif(TARGET gRPC::grpc_cpp_plugin)
        set(GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
    elseif(TARGET grpc_cpp_plugin)
        set(GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
    else()
        find_program(GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin REQUIRED)
    endif()
endif()

set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ui_state.proto)
get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GEN_DIR})

set(PROTO_SRCS
    ${GEN_DIR}/${PROTO_NAME}.pb.cc
    ${GEN_DIR}/${PROTO_NAME}.grpc.pb.cc
)

set(PROTO_HDRS
    ${GEN_DIR}/${PROTO_NAME}.pb.h
    ${GEN_DIR}/${PROTO_NAME}.grpc.pb.h
)

add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS}
    COMMAND ${PROTOC_EXECUTABLE}
    ARGS
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
        --cpp_out=${GEN_DIR}
        --grpc_out=${GEN_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
        ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    VERBATIM
)

add_library(ui_state_proto STATIC
    ${PROTO_SRCS}
)

target_include_directories(ui_state_proto PUBLIC
    ${GEN_DIR}
)

target_link_libraries(ui_state_proto PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
    absl::log_internal_check_op
)

add_executable(server
    main.cpp
)

target_include_directories(server PRIVATE
    ${GEN_DIR}
)

target_link_libraries(server PRIVATE
    ui_state_proto
    protobuf::libprotobuf
    gRPC::grpc++
)

install(TARGETS server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)