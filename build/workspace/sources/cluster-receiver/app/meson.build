pkgconfig = import('pkgconfig')
cpp = meson.get_compiler('cpp')

grpcpp_reflection_dep = cpp.find_library('grpc++_reflection')
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

gstreamer_deps = [
  'gstreamer-1.0', 'gstreamer-allocators-1.0',
  'gstreamer-app-1.0', 'gstreamer-video-1.0',
  'gstreamer-plugins-bad-1.0',
  'gstreamer-plugins-base-1.0',
  'gobject-2.0', 'glib-2.0',
]

deps_remoting = []
foreach depname : gstreamer_deps
  dep = dependency(depname, required: false)
  if not dep.found()
    error('requires @0@ which was not found. '.format(depname))
  endif
  deps_remoting += dep
endforeach



dep_wayland_client = dependency('wayland-client', version: '>= 1.17.0')
dep_scanner = dependency('wayland-scanner', native: true)
prog_scanner = find_program(dep_scanner.get_variable(pkgconfig: 'wayland_scanner'))
dep_wp = dependency('wayland-protocols', version: '>= 1.18')
dir_wp_base = dep_wp.get_variable(pkgconfig: 'pkgdatadir')

cluster_receiver_dep = [
    grpc_deps,
    deps_remoting,
    dep_wayland_client,
    dependency('gio-2.0'),
    dependency('threads'),
    cpp.find_library('gstwayland-1.0'),
]

protocols = [
  { 'name': 'xdg-shell', 'source': 'wp-stable' },
]

foreach proto: protocols
    proto_name = proto['name']
    if proto['source'] == 'wp-stable'
        base_file = proto_name
        xml_path = join_paths(dir_wp_base, 'stable', proto_name, '@0@.xml'.format(base_file))
    else
        base_file = '@0@-unstable-@1@'.format(proto_name, proto['version'])
        xml_path = join_paths(dir_wp_base, 'unstable', proto_name, '@0@.xml'.format(base_file))
    endif

    foreach output_type: [ 'client-header', 'server-header', 'private-code' ]
        if output_type == 'client-header'
            output_file = '@0@-client-protocol.h'.format(base_file)
        elif output_type == 'server-header'
            output_file = '@0@-server-protocol.h'.format(base_file)
        else
            output_file = '@0@-protocol.c'.format(base_file)
            if dep_scanner.version().version_compare('< 1.14.91')
                output_type = 'code'
            endif
        endif

        var_name = output_file.underscorify()
        target = custom_target(
            '@0@ @1@'.format(base_file, output_type),
            command: [ prog_scanner, output_type, '@INPUT@', '@OUTPUT@' ],
            input: xml_path,
            output: output_file,
        )

        set_variable(var_name, target)
    endforeach
endforeach

cluster_receiver_src_headers = [
  'AglShellGrpcClient.h',
  'SerialDbusBridge.h',
  xdg_shell_client_protocol_h,
]

cluster_receiver_src = [
  'AglShellGrpcClient.cpp',
  'SerialDbusBridge.cpp',
  'main.cpp',
  xdg_shell_protocol_c,
  generated_grpc_sources,
  generated_protoc_sources,
]

prefix_path = get_option('prefix')
binplugin_dir = join_paths(prefix_path, get_option('bindir'))

executable('xdg-cluster-receiver',
            sources: [ cluster_receiver_src, cluster_receiver_src_headers ],
            dependencies : cluster_receiver_dep,
            install_rpath: binplugin_dir,
            install: true)
