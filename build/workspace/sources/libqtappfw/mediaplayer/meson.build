qt_dep = dependency('qt6', modules: ['Qml'])

mpdclient_dep = dependency('libmpdclient')

mediaplayer_headers = [ 'MediaplayerBackend.h',
                        'MediaplayerBluezBackend.h',
                        'MediaplayerMpdBackend.h',
                        'MpdEventHandler.h',
                        'mediaplayer.h'
]
moc_files = qt.compile_moc(headers: mediaplayer_headers,
                            dependencies: qt_dep)

src = [ 'MediaplayerBackend.cpp',
        'MediaplayerBluezBackend.cpp',
        'MediaplayerMpdBackend.cpp',
        'MpdEventHandler.cpp',
        'mediaplayer.cpp',
        moc_files
]
lib = shared_library('qtappfw-mediaplayer',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: [qt_dep, mpdclient_dep, qtappfw_bt_dep, qtappfw_vs_dep],
                     install: true)

install_headers('mediaplayer.h')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries: lib,
                 version: '1.0',
                 name: 'libqtappfw-mediaplayer',
                 filebase: 'qtappfw-mediaplayer',
                 requires: 'Qt6Qml',
                 description: 'Library wrapping AGL mediaplayer API in Qt objects')
