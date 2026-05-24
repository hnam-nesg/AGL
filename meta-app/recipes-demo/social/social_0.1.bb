SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = "file://CMakeLists.txt \
           file://main.cpp \
           file://SocialDbus.cpp \
           file://SocialDbus.h \
           file://Social.qml \
           file://HL_Button.qml \
           file://Background.qml \
           file://BorderColor.qml \
           file://ButtonControl.qml \
           file://qml.qrc \
           file://social.desktop \
           file://images/images.qrc \
           file://images/background.png \
           file://images/Roboto-BoldCondensed.ttf \
           file://agl-app@social.service \
           file://org.agl.social.conf \
"  

S = "${WORKDIR}"

inherit qt6-cmake pkgconfig systemd

DEPENDS += " \
    qtbase \
    qtbase-native \
    qtdeclarative \
    qtdeclarative-native \
    qtwebengine \
    qtwebchannel \
    qtpositioning \
    qtvirtualkeyboard \
"

RDEPENDS:${PN} += " \
    qtdeclarative-qmlplugins \
    qtwebengine \
    qtwebengine-qmlplugins \
    qtwebchannel \
    qtwebchannel-qmlplugins \
    qtpositioning \
    qtpositioning-qmlplugins \
    qtvirtualkeyboard \
    qtvirtualkeyboard-qmlplugins \
"
SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "agl-app@social.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/social ${D}${bindir}/social

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/social.desktop \
        ${D}${datadir}/applications/social.desktop
    
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@social.service ${D}${systemd_system_unitdir}/

    install -d ${D}${datadir}/dbus-1/system.d
    install -m 0644 ${WORKDIR}/org.agl.social.conf \
        ${D}${datadir}/dbus-1/system.d/org.agl.social.conf
}

FILES:${PN} += " \
    ${bindir}/social \
    ${datadir}/applications/social.desktop \
    ${systemd_system_unitdir}/agl-app@social.service \
    ${datadir}/dbus-1/system.d/org.agl.social.conf \
"