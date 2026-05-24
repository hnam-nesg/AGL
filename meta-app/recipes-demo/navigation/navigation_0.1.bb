SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = "file://files.tar.gz \
            file://agl-app@navigation.service \
            file://navigation.desktop \
            file://org.agl.navigation.conf \
"

S = "${WORKDIR}/files"

inherit qt6-cmake pkgconfig systemd

DEPENDS += " \
    qtbase \
    qtbase-native \
    qtdeclarative \
    qtdeclarative-native \
    qtpositioning \
    qtlocation \
    qtwebengine \
    qtwebchannel \
    qtvirtualkeyboard \
    sqlite3 \
"

RDEPENDS:${PN} += " \
    qtdeclarative-qmlplugins \
    qtwayland \
    qtbase-qmlplugins \
    qtlocation \
    qtlocation-qmlplugins \
    qtlocation-plugins \
    qtwebengine \
    qtwebengine-qmlplugins \
    qtwebchannel \
    qtwebchannel-qmlplugins \
    qtvirtualkeyboard \
    qtvirtualkeyboard-qmlplugins \
    qtquickcontrols2-agl-style \
    sqlite3 \
"

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "agl-app@navigation.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

RPROVIDES:${PN} += "ondemandnavi"
RREPLACES:${PN} += "ondemandnavi"
RCONFLICTS:${PN} += "ondemandnavi"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/navigation ${D}${bindir}/navigation

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/navigation.desktop \
        ${D}${datadir}/applications/navigation.desktop

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@navigation.service ${D}${systemd_system_unitdir}/

    install -d ${D}${datadir}/dbus-1/system.d
    install -m 0644 ${WORKDIR}/org.agl.navigation.conf \
        ${D}${datadir}/dbus-1/system.d/org.agl.navigation.conf

    install -d ${D}${datadir}/navigation/maps

    install -d ${D}${sysconfdir}/systemd/system/multi-user.target.wants
    ln -sf ${systemd_system_unitdir}/agl-app@navigation.service \
        ${D}${sysconfdir}/systemd/system/multi-user.target.wants/agl-app@navigation.service
}

FILES:${PN} += " \
    ${bindir}/navigation \
    ${datadir}/applications/navigation.desktop \
    ${systemd_system_unitdir}/agl-app@navigation.service \
    ${sysconfdir}/systemd/system/multi-user.target.wants/agl-app@navigation.service \
    ${datadir}/dbus-1/system.d/org.agl.navigation.conf \
    ${datadir}/navigation/maps \
"
