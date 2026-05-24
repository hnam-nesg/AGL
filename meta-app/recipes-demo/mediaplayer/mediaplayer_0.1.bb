SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = "file://files.tar.gz \
            file://mediaplayer.desktop \
            file://agl-app@mediaplayer.service \
            file://org.agl.media.conf \
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
    libqtappfw mpd \
"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "agl-app@mediaplayer.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/mediaplayer ${D}${bindir}/mediaplayer

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/mediaplayer.desktop \
        ${D}${datadir}/applications/mediaplayer.desktop

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@mediaplayer.service ${D}${systemd_system_unitdir}/

    install -d ${D}${datadir}/dbus-1/system.d
    install -m 0644 ${WORKDIR}/org.agl.media.conf \
        ${D}${datadir}/dbus-1/system.d/org.agl.media.conf
}

FILES:${PN} += " \
    ${bindir}/mediaplayer \
    ${datadir}/applications/mediaplayer.desktop \
    ${systemd_system_unitdir}/agl-app@mediaplayer.service \
    ${datadir}/dbus-1/system.d/org.agl.media.conf \
"
