SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = "file://CMakeLists.txt \
           file://main.cpp \
           file://ui_state.proto \
           file://server.desktop \
           file://agl-app@server.service \
"  

S = "${WORKDIR}"

DEPENDS = "grpc protobuf grpc-native protobuf-native abseil-cpp"

inherit cmake pkgconfig systemd

EXTRA_OECMAKE += " \
    -DPROTOC_EXECUTABLE=${STAGING_BINDIR_NATIVE}/protoc \
    -DGRPC_CPP_PLUGIN_EXECUTABLE=${STAGING_BINDIR_NATIVE}/grpc_cpp_plugin \
"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "agl-app@server.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/server ${D}${bindir}/server

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/server.desktop \
        ${D}${datadir}/applications/server.desktop
    
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@server.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += " \
    ${bindir}/server \
    ${datadir}/applications/server.desktop \
    ${systemd_system_unitdir}/agl-app@server.service \
"