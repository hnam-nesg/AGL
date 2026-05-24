SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = "file://files.tar.gz \
"  

S = "${WORKDIR}/files"

inherit qt6-cmake pkgconfig systemd

DEPENDS += " \
    qtbase \
    qtbase-native \
    qtdeclarative \
    qtdeclarative-native \
    qtquick3d \
    qtquick3d-native \
"

RDEPENDS:${PN} += " \
    qtdeclarative-qmlplugins \
    qtwayland \
    qtbase-qmlplugins \
    qt5compat \
    qtquickcontrols2-agl-style \
    qtquick3d \
"
SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "agl-app@light.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/light ${D}${bindir}/light

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/light.desktop \
        ${D}${datadir}/applications/light.desktop

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@light.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += " \
    ${bindir}/light \
    ${datadir}/applications/light.desktop \
    ${systemd_system_unitdir}/agl-app@light.service \
"