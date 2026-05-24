SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = "file://files.tar.gz \
            file://hvac.desktop \
            file://agl-app@hvac.service \
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
    qtshadertools \
    qtshadertools-native \
    libqtappfw \
"
RDEPENDS:${PN} += " \
    qtdeclarative-qmlplugins \
    qtwayland \
    qtbase-qmlplugins \
    qt5compat \
    qtquickcontrols2-agl-style \
    qtquick3d \
    libqtappfw \
"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "agl-app@hvac.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/hvac ${D}${bindir}/hvac

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/hvac.desktop \
        ${D}${datadir}/applications/hvac.desktop

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@hvac.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += " \
    ${bindir}/hvac \
    ${datadir}/applications/hvac.desktop \
    ${systemd_system_unitdir}/agl-app@hvac.service \
"