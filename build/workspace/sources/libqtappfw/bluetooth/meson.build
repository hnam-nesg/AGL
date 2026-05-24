qt_dep = dependency('qt6', modules: ['Core', 'Qml'])

glib_dep = [dependency('glib-2.0'), dependency('gio-2.0'), dependency('gobject-2.0'), dependency('gio-unix-2.0')]
bluez_glib_dep = dependency('bluez-glib')

moc_files = qt.compile_moc(headers : ['bluetooth.h', 'bluetoothmodel.h'],
                            dependencies: qt_dep)

src = ['bluetooth.cpp', 'bluetoothmodel.cpp', 'bluetootheventhandler.cpp', moc_files]
lib = shared_library('qtappfw-bt',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: [qt_dep, glib_dep, bluez_glib_dep],
                     install: true)

install_headers('bluetooth.h')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : lib,
                 version : '1.0',
                 name : 'libqtappfw-bt',
                 filebase : 'qtappfw-bt',
                 requires: 'Qt6Qml',
                 description : 'AGL Qt demo app framework helper library for using BlueZ.')

qtappfw_bt_dep = declare_dependency(link_with: lib,
                                    include_directories: '.',
                                    sources: ['bluetooth.h'])
