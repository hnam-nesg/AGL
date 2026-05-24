SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = "file://CMakeLists.txt \
           file://src/main.cpp \
           file://qml/Mirror.qml \
           file://qml/HL_Button.qml \
           file://qml/Background.qml \
           file://agl-mirror.desktop \
           file://qml/images/background.png \
           file://qml/images/mirror-left.png \
           file://qml/images/mirror-right.png \
           file://qml/images/agl-mirror.png \
           file://qml/images/Roboto-BoldCondensed.ttf \
           file://qml/qml.qrc \
           file://qml/images/images.qrc \
           file://qml/Background.qml \
           file://qml/BorderColor.qml \
           file://qml/ButtonControl.qml \
           file://agl-app@agl-mirror.service \
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
SYSTEMD_SERVICE:${PN} = "agl-app@agl-mirror.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/agl-mirror ${D}${bindir}/agl-mirror

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/agl-mirror.desktop \
        ${D}${datadir}/applications/agl-mirror.desktop
    
    install -d ${D}${datadir}/icons/hicolor/128x128/apps
    install -m 0644 ${WORKDIR}/qml/images/agl-mirror.png ${D}${datadir}/icons/hicolor/128x128/apps/agl-mirror.png

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@agl-mirror.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += " \
    ${bindir}/agl-mirror \
    ${datadir}/applications/agl-mirror.desktop \
    ${datadir}/icons/hicolor/128x128/apps/agl-mirror.png \
    ${systemd_system_unitdir}/agl-app@agl-mirror.service \
"