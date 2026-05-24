dep_wayland_client = dependency('wayland-client', version: '>= 1.17.0')

grpcpp_reflection_dep = cxx.find_library('grpc++_reflection')
protoc = find_program('protoc')
grpc_cpp = find_program('grpc_cpp_plugin')

protoc_gen = generator(protoc, \
                       output : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
                       arguments : ['--proto_path=@CURRENT_SOURCE_DIR@/',
                         '--cpp_out=@BUILD_DIR@',
                         '@INPUT@'])

generated_protoc_sources = protoc_gen.process('agl_shell.proto')

grpc_gen = generator(protoc, \
                     output : ['@BASENAME@.grpc.pb.cc', '@BASENAME@.grpc.pb.h'],
                     arguments : ['--proto_path=@CURRENT_SOURCE_DIR@/',
                       '--grpc_out=@BUILD_DIR@',
                       '--plugin=protoc-gen-grpc=' + grpc_cpp.path(),
                       '@INPUT@'])
generated_grpc_sources = grpc_gen.process('agl_shell.proto')

grpc_deps = [
    dependency('protobuf'),
    dependency('grpc'),
    dependency('grpc++'),
    grpcpp_reflection_dep,
]

srcs = [
      'main-grpc.cpp',
      'grpc-async-cb.cpp',
      'shell.cpp',
      generated_protoc_sources,
      generated_grpc_sources,
      agl_shell_client_protocol_h,
      agl_shell_protocol_c,
]

executable(
    'agl-shell-grpc-server', srcs,
    include_directories: [ common_inc ],
    dependencies: [ grpc_deps, dep_wayland_client, libweston_dep ],
    install: true,
    install_dir: dir_module_agl_compositor,
)
