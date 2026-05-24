SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = "file://files.tar.gz \
            file://settings.desktop \
            file://agl-app@settings.service \
"  

S = "${WORKDIR}/files"

inherit qt6-cmake pkgconfig systemd

DEPENDS += " \
    qtbase \
    qtbase-native \
    qtdeclarative \
    qtdeclarative-native \
    libqtappfw \
"
RDEPENDS:${PN} += " \
    qtdeclarative-qmlplugins \
    qtwayland \
    qtbase-qmlplugins \
    qt5compat \
    qtquickcontrols2-agl-style \
    libqtappfw \
"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "agl-app@settings.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

RPROVIDES:${PN} += "settings-old"
RREPLACES:${PN} += "settings-old"
RCONFLICTS:${PN} += "settings-old"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/settings ${D}${bindir}/settings

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/settings.desktop \
        ${D}${datadir}/applications/settings.desktop

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@settings.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += " \
    ${bindir}/settings \
    ${datadir}/applications/settings.desktop \
    ${systemd_system_unitdir}/agl-app@settings.service \
"