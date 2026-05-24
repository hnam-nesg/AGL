SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = " file://files.tar.gz \
            file://agl-app@phone.service \
            file://phone.desktop \
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
    qtquickcontrols2-agl-style \
"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "agl-app@phone.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/phone ${D}${bindir}/phone

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/phone.desktop \
        ${D}${datadir}/applications/phone.desktop

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@phone.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += " \
    ${bindir}/phone \
    ${datadir}/applications/phone.desktop \
    ${systemd_system_unitdir}/agl-app@phone.service \
"