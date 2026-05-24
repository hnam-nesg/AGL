SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = "file://files.tar.gz \
        file://drivemode.desktop \
        file://agl-app@drivemode.service \
        file://CMakeLists.txt \
"

S = "${WORKDIR}/files"

inherit qt6-cmake pkgconfig systemd

DEPENDS += " \
    qtbase \
    qtbase-native \
    qtdeclarative \
    qtdeclarative-native \
"

RDEPENDS:${PN} += " \
    qtdeclarative-qmlplugins \
    qtwayland \
    qtbase-qmlplugins \
    qt5compat \
    qtquickcontrols2-agl-style \
"
SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "agl-app@drivemode.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/drivemode ${D}${bindir}/drivemode

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/drivemode.desktop \
        ${D}${datadir}/applications/drivemode.desktop

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@drivemode.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += " \
    ${bindir}/drivemode \
    ${datadir}/applications/drivemode.desktop \
    ${systemd_system_unitdir}/agl-app@drivemode.service \
"