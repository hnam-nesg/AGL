cpp = meson.get_compiler('cpp')
grpcpp_reflection_dep = cpp.find_library('grpc++_reflection')

qt_dep = dependency('qt6', modules: ['Qml'])
applauncher_dep = [
    qt_dep,
    dependency('protobuf'),
    dependency('grpc'),
    dependency('grpc++'),
    grpcpp_reflection_dep,
]

protoc = find_program('protoc')
grpc_cpp = find_program('grpc_cpp_plugin')

protoc_gen = generator(protoc, \
                       output : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
                       arguments : ['--proto_path=@CURRENT_SOURCE_DIR@/protos',
                         '--cpp_out=@BUILD_DIR@',
                         '@INPUT@'])
generated_protoc_sources = protoc_gen.process('protos/applauncher.proto')

grpc_gen = generator(protoc, \
                     output : ['@BASENAME@.grpc.pb.cc', '@BASENAME@.grpc.pb.h'],
                     arguments : ['--proto_path=@CURRENT_SOURCE_DIR@/protos',
                       '--grpc_out=@BUILD_DIR@',
                       '--plugin=protoc-gen-grpc=' + grpc_cpp.full_path(),
                       '@INPUT@'])
generated_grpc_sources = grpc_gen.process('protos/applauncher.proto')

moc_files = qt.compile_moc(headers : ['AppLauncherClient.h', 'AppLauncherGrpcClient.h'],
                            dependencies: qt_dep)

src = [
    'AppLauncherClient.cpp',
    'AppLauncherGrpcClient.cpp',
    generated_protoc_sources,
    generated_grpc_sources,
    moc_files
]
lib = shared_library('qtappfw-applauncher',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: applauncher_dep,
                     install: true)

install_headers('AppLauncherClient.h')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : lib,
                 version : '1.0',
                 name : 'libqtappfw-applauncher',
                 filebase : 'qtappfw-applauncher',
                 requires: 'Qt6Qml',
                 description : 'Library wrapping AGL AppLauncher API in Qt objects')
