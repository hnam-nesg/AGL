qt_dep = dependency('qt6', modules: ['Qml'])

glib_dep = [dependency('glib-2.0'), dependency('gio-2.0'), dependency('gobject-2.0'), dependency('gio-unix-2.0')]
connman_glib_dep = dependency('connman-glib')

network_headers = [ 'abstractnetworkmodel.h',
                    'network.h',
                    'networkadapter.h',
                    'networkeventhandler.h',
                    'wifiadapter.h',
                    'wifinetworkmodel.h',
                    'wiredadapter.h',
                    'wirednetworkmodel.h'
]
moc_files = qt.compile_moc(headers: network_headers,
                            dependencies: qt_dep)

src = [ 'network.cpp',
        'networkeventhandler.cpp',
        'wifiadapter.cpp',
        'wiredadapter.cpp',
        'wifinetworkmodel.cpp',
        'wirednetworkmodel.cpp',
        'abstractnetworkmodel.cpp',
        'connectionprofile.cpp',
        moc_files,
]
lib = shared_library('qtappfw-network',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: [qt_dep, glib_dep, connman_glib_dep],
                     install: true)

install_headers(['network.h', 'networkadapter.h', 'wifiadapter.h', 'wiredadapter.h'])

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : lib,
                 version : '1.0',
                 name : 'libqtappfw-network',
                 filebase : 'qtappfw-network',
                 requires: 'Qt6Qml',
                 description : 'Library wrapping AGL network API in Qt objects')
