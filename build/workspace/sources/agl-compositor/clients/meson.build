dep_wayland_client = dependency('wayland-client', version: '>= 1.17.0')
dep_pixman = dependency('pixman-1', version: '>= 0.25.2')

clients = [
{
	'basename': 'agl-screenshooter',
	'sources': [
	  'screenshot.c',
	  '../shared/file-util.c',
	  '../shared/os-compatibility.c',
	  '../shared/xalloc.c',
	  weston_output_capture_protocol_c,
	  weston_output_capture_client_protocol_h,
	],
	'deps_objs' : [ dep_wayland_client, dep_pixman ],
	'deps': [ 'cairo', ],
},
]

foreach t: clients
  t_name = t.get('basename')
  t_deps = t.get('deps_objs', [])

  foreach xdep: t.get('deps', [])
    dep = dependency(xdep, required: false)
    if dep.found()
      t_deps += dep
    endif
  endforeach

  executable(
      t_name, t.get('sources'),
      include_directories: [ common_inc ],
      dependencies: [ t_deps, libweston_dep ],
      install: true,
  )

  message('Building client ' + t_name)
endforeach

install_data('agl-stream-pipewire-output', install_dir: get_option('bindir'))
