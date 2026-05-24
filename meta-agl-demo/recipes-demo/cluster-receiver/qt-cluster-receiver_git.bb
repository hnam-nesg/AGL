SUMMARY     = "Instrument Cluster receiver application"
DESCRIPTION = "AGL demonstration instrument cluster Qt remote display application"
HOMEPAGE    = "https://gerrit.automotivelinux.org/gerrit/admin/repos/apps/agl-qt-cluster-demo-receiver"
SECTION     = "apps"

LICENSE     = "Apache-2.0 & MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=ae6497158920d9524cf208c09cc4c984 \
                    file://app/surface.hpp;beginline=5;endline=21;md5=5351c531a191f0e3463aafcd0a6a00a3"

DEPENDS = " \
    wayland wayland-native \
    qtwayland qtwayland-native \
    qtdeclarative qtwebsockets qtbase qtdeclarative \
    gstreamer1.0 gstreamer1.0-plugins-base gstreamer1.0-plugins-bad \
"

PV = "1.0+git${SRCPV}"

SRC_URI = "git://gerrit.automotivelinux.org/gerrit/apps/agl-qt-cluster-demo-receiver;protocol=https;branch=${AGL_BRANCH} \
           file://qt-cluster-receiver.service \
"
SRCREV  = "fee1297d91fcb499b49bcc4c0d9690f5d92ce2d4"

S  = "${WORKDIR}/git"

inherit qt6-cmake pkgconfig systemd

do_install:append() {
    # Only install unit, do not enable it by default
    install -D -m 0644 ${WORKDIR}/${BPN}.service ${D}${systemd_system_unitdir}/${BPN}.service
}

RDEPENDS:${PN} += " \
    qtwayland \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
"

SYSTEMD_SERVICE:${PN} = "${BPN}.service"
SYSTEMD_AUTO_ENABLE:${PN} = "disable"
