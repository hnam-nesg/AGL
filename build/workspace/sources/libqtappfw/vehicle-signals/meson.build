cpp = meson.get_compiler('cpp')
grpcpp_reflection_dep = cpp.find_library('grpc++_reflection')

qt_dep = dependency('qt6', modules: ['Core', 'Concurrent'])

vs_dep = [
    qt_dep,
    dependency('protobuf'),
    dependency('grpc'),
    dependency('grpc++'),
    grpcpp_reflection_dep,
]

protoc = find_program('protoc')
grpc_cpp = find_program('grpc_cpp_plugin')

protos_base_dir = get_option('protos')
protos_dir = protos_base_dir / 'kuksa/val/v1'
protoc_gen = generator(protoc, \
                       output : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
                       arguments : ['-I=' + protos_base_dir,
                         '--cpp_out=@BUILD_DIR@',
                         '@INPUT@'])
generated_protoc_sources = [ \
    protoc_gen.process(protos_dir / 'types.proto', preserve_path_from : protos_base_dir),
    protoc_gen.process(protos_dir / 'val.proto', preserve_path_from : protos_base_dir),
]

grpc_gen = generator(protoc, \
                     output : ['@BASENAME@.grpc.pb.cc', '@BASENAME@.grpc.pb.h'],
                     arguments : ['-I=' + protos_base_dir,
                       '--grpc_out=@BUILD_DIR@',
                       '--plugin=protoc-gen-grpc=' + grpc_cpp.full_path(),
                       '@INPUT@'])
generated_grpc_sources = [ \
    grpc_gen.process(protos_dir / 'val.proto', preserve_path_from : protos_base_dir),
]

moc_files = qt.compile_moc(headers: [ 'vehiclesignals.h', 'QtKuksaClient.h' ],
                            dependencies: qt_dep)

src = [
    'vehiclesignals.cpp',
    'VehicleSignalsConfig.cpp',
    'QtKuksaClient.cpp',
    moc_files,
    generated_protoc_sources,
    generated_grpc_sources,
]

lib = shared_library('qtappfw-vehicle-signals',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: vs_dep,
                     install: true)

install_headers(['vehiclesignals.h', 'VehicleSignalsConfig.h'])

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries: lib,
                 version: '1.0',
                 name: 'libqtappfw-vehicle-signals',
                 filebase: 'qtappfw-vehicle-signals',
                 requires: ['Qt6Core'],
                 description: 'Library wrapping VSS API in Qt objects')

qtappfw_vs_dep = declare_dependency(dependencies: qt_dep,
                                    link_with: lib,
                                    include_directories: '.',
                                    sources: ['vehiclesignals.h'])
