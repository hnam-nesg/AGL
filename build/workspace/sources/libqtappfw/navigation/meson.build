qt_dep = dependency('qt6', modules: ['Qml'])

moc_files = qt.compile_moc(headers: 'navigation.h',
                            dependencies: qt_dep)

src = ['navigation.cpp', moc_files]
lib = shared_library('qtappfw-navigation',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: [qt_dep, qtappfw_vs_dep],
                     install: true)

install_headers('navigation.h')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : lib,
                 version : '1.0',
                 name : 'libqtappfw-navigation',
                 filebase : 'qtappfw-navigation',
                 requires: 'Qt6Qml',
                 description : 'Library wrapping AGL navigation API in Qt objects')
