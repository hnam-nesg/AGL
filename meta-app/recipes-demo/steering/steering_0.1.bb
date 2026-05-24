SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = "file://CMakeLists.txt \
           file://main.cpp \
           file://Steering.qml \
           file://HL_Button.qml \
           file://Background.qml \
           file://BorderColor.qml \
           file://ButtonControl.qml \
           file://qml.qrc \
           file://steering.desktop \
           file://images/images.qrc \
           file://images/background.png \
           file://images/Roboto-BoldCondensed.ttf \
           file://agl-app@steering.service \
"  

S = "${WORKDIR}"

inherit qt6-cmake pkgconfig systemd

DEPENDS += " \
    qtbase \
    qtbase-native \
    qtdeclarative \
    qtdeclarative-native \
"

RDEPENDS:${PN} += " \
    qtdeclarative-qmlplugins \
"
SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "agl-app@steering.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/steering ${D}${bindir}/steering

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/steering.desktop \
        ${D}${datadir}/applications/steering.desktop
    
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@steering.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += " \
    ${bindir}/steering \
    ${datadir}/applications/steering.desktop \
    ${systemd_system_unitdir}/agl-app@steering.service \
"