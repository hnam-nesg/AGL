qt_dep = dependency('qt6', modules: ['Qml'])

moc_files = qt.compile_moc(headers: 'hvac.h',
                            dependencies: qt_dep)

src = ['hvac.cpp', moc_files]
lib = shared_library('qtappfw-hvac',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: [qt_dep, qtappfw_vs_dep],
                     install: true)

install_headers('hvac.h')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : lib,
                 version : '1.0',
                 name : 'libqtappfw-hvac',
                 filebase : 'qtappfw-hvac',
                 requires: 'Qt6Qml',
                 description : 'AGL Qt demo app framework helper library for using HVAC.')
