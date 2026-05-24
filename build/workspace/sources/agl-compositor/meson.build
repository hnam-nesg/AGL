project('agl-compositor',
  'c','cpp',
  version: '0.0.24',
  default_options: [
    'warning_level=3',
    'c_std=gnu99',
    'werror=true',
  ],
  meson_version: '>= 1.0',
  license: 'MIT/Expat',
)

config_h = configuration_data()
libweston_version = 'libweston-13'
pkgconfig = import('pkgconfig')
fs = import('fs')

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

add_project_arguments(
  cc.get_supported_arguments([
    '-Wno-unused-parameter',
    '-Wno-pedantic',
    '-Wno-deprecated-declarations'
  ]),
  language: 'c'
)

add_project_arguments([
    '-DPACKAGE_STRING="agl-compositor @0@"'.format(meson.project_version()),
    '-D_GNU_SOURCE',
    '-D_ALL_SOURCE',
  ],
  language: 'c'
)

optional_libc_funcs = [ 'memfd_create', 'strchrnul' ]
foreach func: optional_libc_funcs
    if cc.has_function(func)
        add_project_arguments('-DHAVE_@0@=1'.format(func.to_upper()), language: 'c')
    endif
endforeach

dep_libsystemd = dependency('libsystemd', required: false)
dep_scanner = dependency('wayland-scanner', native: true)
prog_scanner = find_program(dep_scanner.get_variable(pkgconfig: 'wayland_scanner'))
dep_wp = dependency('wayland-protocols', version: '>= 1.18')
dir_wp_base = dep_wp.get_variable(pkgconfig: 'pkgdatadir')

depnames = [
  'gstreamer-1.0', 'gstreamer-allocators-1.0',
  'gstreamer-app-1.0', 'gstreamer-video-1.0',
  'gobject-2.0', 'glib-2.0'
]

deps_remoting = []
foreach depname : depnames
  dep = dependency(depname, required: false)
  if not dep.found()
    message('Remoting requires @0@ which was not found. '.format(depname))
  endif
  deps_remoting += dep
endforeach

agl_shell_xml = files('protocol/agl-shell.xml')
xdg_shell_xml = join_paths(dir_wp_base, 'stable', 'xdg-shell', 'xdg-shell.xml')
dep_libweston_protocols = dependency('libweston-13-protocols', version: '>= 13')
dir_protocol_libweston = dep_libweston_protocols.get_pkgconfig_variable('pkgdatadir')

protocols = [
  { 'name': 'agl-shell', 'source': 'internal' },
  { 'name': 'xdg-shell', 'source': 'wp-stable' },
  { 'name': 'xdg-output', 'source': 'unstable', 'version': 'v1' },
  { 'name': 'weston-output-capture', 'source': 'libweston-protocols' },
]

foreach proto: protocols
    proto_name = proto['name']
    if proto['source'] == 'internal'
        base_file = proto_name
        xml_path = join_paths('protocol', '@0@.xml'.format(base_file))
    elif proto['source'] == 'libweston-protocols'
        base_file = proto_name
        xml_path = join_paths(dir_protocol_libweston, '@0@.xml'.format(base_file))
    elif proto['source'] == 'wp-stable'
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

prefix_path = get_option('prefix')
dir_data = join_paths(prefix_path, get_option('datadir'))
dir_data_agl_compositor = join_paths('agl-compositor', 'protocols')
dir_data_pc = join_paths(dir_data, 'pkgconfig')
libweston_dep = dependency(libweston_version)

deps_libweston = [
  dependency('wayland-server'),
  dependency('weston'),
  libweston_dep,
]

weston_module_dir = libweston_dep.get_pkgconfig_variable('libdir')
config_h.set_quoted('WESTON_MODULEDIR', join_paths(weston_module_dir, 'weston'))
config_h.set_quoted('LIBWESTON_MODULEDIR', join_paths(weston_module_dir, libweston_version))

srcs_agl_compositor = [
	'src/compositor.c',
	'src/desktop.c',
	'src/layout.c',
	'src/policy.c',
	'src/shell.c',
	'src/input.c',
	'src/drm-lease.c',
	'shared/option-parser.c',
	'shared/os-compatibility.c',
	'shared/process-util.c',
	agl_shell_server_protocol_h,
	agl_shell_protocol_c,
	xdg_shell_protocol_c,
]

policy_to_install = get_option('policy-default')
if policy_to_install == 'auto' or policy_to_install == 'allow-all'
  srcs_agl_compositor += 'src/policy-default.c'
  message('Installing allow all policy')
elif policy_to_install == 'deny-all'
  srcs_agl_compositor += 'src/policy-deny.c'
  message('Installing deny all policy')
elif policy_to_install == 'rba'
  srcs_agl_compositor += ['src/policy-rba.c', 'src/rba_adapter.cpp']
  deps_libweston += dependency('librba')
  message('Installing rba policy')
endif

if get_option('drm-lease')
  deps_libweston += dependency('libdlmclient')
  config_h.set('HAVE_DRM_LEASE', '1')
endif

# From meson documentation:
# In order to look for headers in a specific directory you can use args :
# '-I/extra/include/dir, but this should only be used in exceptional cases for
# includes that can't be detected via pkg-config and passed via dependencies.
if libweston_dep.found()
  if not meson.is_cross_build()
    if not prefix_path.contains('/usr')
      dir_path_x11_backend = join_paths(prefix_path, 'include', libweston_version, 'libweston', 'backend-x11.h')
      dir_path_wayland_backend = join_paths(prefix_path, 'include', libweston_version, 'libweston', 'backend-wayland.h')
      dir_path_rdp_backend = join_paths(prefix_path, 'include', libweston_version, 'libweston', 'backend-rdp.h')
      dir_path_pipewire_backend = join_paths(prefix_path, 'include', libweston_version, 'libweston', 'backend-pipewire.h')
    else
      dir_path_x11_backend = join_paths(libweston_version, 'libweston', 'backend-x11.h')
      dir_path_wayland_backend = join_paths(libweston_version, 'libweston', 'backend-wayland.h')
      dir_path_rdp_backend = join_paths(libweston_version, 'libweston', 'backend-rdp.h')
      dir_path_pipewire_backend = join_paths(libweston_version, 'libweston', 'backend-pipewire.h')
    endif
  else
    message('Building with cross environment')
    dir_path_x11_backend = join_paths(libweston_version, 'libweston', 'backend-x11.h')
    dir_path_wayland_backend = join_paths(libweston_version, 'libweston', 'backend-wayland.h')
    dir_path_rdp_backend = join_paths(libweston_version, 'libweston', 'backend-rdp.h')
    dir_path_pipewire_backend = join_paths(libweston_version, 'libweston', 'backend-pipewire.h')
  endif

  # do the test
  if cc.has_header(dir_path_x11_backend)
    config_h.set('HAVE_BACKEND_X11', 1)
    message('Building with X11 backend')
  endif

  if cc.has_header(dir_path_wayland_backend)
    config_h.set('HAVE_BACKEND_WAYLAND', 1)
    message('Building with Wayland backend')
  endif

  if cc.has_header(dir_path_rdp_backend)
    config_h.set('HAVE_BACKEND_RDP', 1)
    message('Building with RDP backend')
  endif
  if cc.has_header(dir_path_pipewire_backend)
    config_h.set('HAVE_BACKEND_PIPEWIRE', 1)
    message('Building with PipeWire backend')
  endif
endif

if dep_libsystemd.found()
  config_h.set('HAVE_SYSTEMD', 1)

  srcs_agl_compositor += 'src/systemd-notify.c'
  deps_libweston += dep_libsystemd

  message('Found systemd, enabling notify support')
endif

if deps_remoting.length() == depnames.length()
  config_h.set('HAVE_REMOTING', 1)
  message('Found remoting depends, enabling remoting')
endif

if get_option('xwayland')
        config_h.set('BUILD_XWAYLAND', '1')

        srcs_agl_compositor += 'src/xwayland.c'
        srcs_agl_compositor += 'shared/xalloc.c'
        srcs_agl_compositor += 'shared/process-util.c'
        config_h.set_quoted('XSERVER_PATH', get_option('xwayland-path'))
endif

dir_module_agl_compositor = join_paths(join_paths(prefix_path, get_option('libdir')), 'agl-compositor')

libexec_compositor = shared_library(
        'exec_compositor',
        sources: srcs_agl_compositor,
        dependencies: deps_libweston,
        install_dir: dir_module_agl_compositor,
        install: true,
        version: meson.project_version(),
        soversion: 0
)

dep_libexec_compositor = declare_dependency(
        link_with: libexec_compositor,
        include_directories: [ include_directories('.') ],
        dependencies: deps_libweston,
)

configure_file(output: 'config.h', configuration: config_h)

exe_agl_compositor = executable(
	'agl-compositor',
	'src/main.c',
	dependencies: dep_libexec_compositor,
	install_rpath: dir_module_agl_compositor,
	install: true
)

pkgconfig.generate(
        filebase: 'agl-compositor-@0@-protocols'.format(meson.project_version()),
        name: 'agl-compositor private protocols',
        version: meson.project_version(),
        description: 'agl-compositor protocol files',
        variables: [
                'datarootdir=' + join_paths('${prefix}', get_option('datadir')),
                'pkgdatadir=' + join_paths('${pc_sysrootdir}${datarootdir}', dir_data_agl_compositor)
        ],
        install_dir: dir_data_pc
)

install_data(
        [ agl_shell_xml ],
        install_dir: join_paths(dir_data, dir_data_agl_compositor)
)

common_inc = [ include_directories('src'), include_directories('.') ]
subdir('clients')

if get_option('grpc-proxy')
  subdir('grpc-proxy')
endif
