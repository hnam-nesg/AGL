SUMMARY = "Simple example service for AGL"
DESCRIPTION = "A simple background service for Automotive Grade Linux"
LICENSE = "CLOSED"

SRC_URI = "file://files.tar.gz \
        file://assistant.desktop \
        file://agl-app@assistant.service \
        file://CMakeLists.txt \
"

S = "${WORKDIR}/files"

inherit qt6-cmake pkgconfig systemd

DEPENDS += " \
    qtbase \
    qtbase-native \
    qtdeclarative \
    qtdeclarative-native \
    whisper-cpp \
    qtmultimedia \
    qtshadertools \
    qtshadertools-native \
    wakeword \
    alsa-lib \
    rnnoise \
    sherpa-onnx \
    libqtappfw \
    llama.cpp \
"

RDEPENDS:${PN} += " \
    qtdeclarative-qmlplugins \
    qtwayland \
    qtbase-qmlplugins \
    qt5compat \
    qtquickcontrols2-agl-style \
    qtmultimedia \
    qtmultimedia-plugins \
    alsa-plugins \
    pipewire-alsa \
    gstreamer1.0-plugins-base-alsa \
    gstreamer1.0-plugins-base-audioconvert \
    gstreamer1.0-plugins-base-audioresample \
    gstreamer1.0-pipewire \
    wakeword \
    rnnoise \
    libqtappfw \
    sherpa-onnx \
    llama.cpp \
"
SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE:${PN} = "agl-app@assistant.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/assistant ${D}${bindir}/assistant

    install -d ${D}${datadir}/applications
    install -m 0644 ${WORKDIR}/assistant.desktop \
        ${D}${datadir}/applications/assistant.desktop

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/agl-app@assistant.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += " \
    ${bindir}/assistant \
    ${datadir}/applications/assistant.desktop \
    ${systemd_system_unitdir}/agl-app@assistant.service \
"
