qt_dep = dependency('qt6', modules: ['Qml'])

moc_files = qt.compile_moc(headers: 'telephony.h',
                            dependencies: qt_dep)

src = ['telephony.cpp', moc_files]
lib = shared_library('qtappfw-phone',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: qt_dep,
                     install: true)

install_headers('telephony.h')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : lib,
                 version : '1.0',
                 name : 'libqtappfw-phone',
                 filebase : 'qtappfw-phone',
                 requires: 'Qt6Qml',
                 description : 'Library wrapping AGL telephony API in Qt objects')
