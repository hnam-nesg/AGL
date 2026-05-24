SUMMARY     = "Instrument Cluster Dashboard application"
DESCRIPTION = "AGL demonstration instrument cluster dashboard application"
HOMEPAGE    = "https://gerrit.automotivelinux.org/gerrit/#/admin/projects/apps/agl-cluster-demo-dashboard"
SECTION     = "apps"

LICENSE     = "Apache-2.0 & BSD-3-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=ae6497158920d9524cf208c09cc4c984 \
                    file://app/images/carbon_icons/LICENSE.md;md5=006ab90afbfba5ead6131a668b2776df \
                    file://app/cluster-dashboard.qml;beginline=9;endline=48;md5=54187d50b29429abee6095fe8b7c1a78"

DEPENDS = " \
    qtdeclarative \
    libqtappfw \
    glib-2.0 \
    wayland \
    wayland-native \
    qtdeclarative-native \
    qtpositioning \
    qtlocation \
    qtwebengine \
    qtwebchannel \
    sqlite3 \
"

PV = "1.0+git${SRCPV}"

SRC_URI = "git://gerrit.automotivelinux.org/gerrit/apps/agl-cluster-demo-dashboard;protocol=https;branch=${AGL_BRANCH} \
           file://cluster-dashboard.service \
           file://cluster-dashboard.conf.default \
           file://cluster-dashboard.conf.demo \
           file://cluster-dashboard.token \
"
SRCREV  = "034c412264d3a51e1d400976225a783a1e15d9a2"

S  = "${WORKDIR}/git"

inherit pkgconfig qt6-cmake update-alternatives systemd

CLUSTER_DEMO_VSS_HOSTNAME ??= "192.168.10.2"

SYSTEMD_SERVICE:${PN} = "${BPN}.service"

do_install:append() {
    install -D -m 0644 ${WORKDIR}/${BPN}.service ${D}${systemd_system_unitdir}/${BPN}.service

    # VIS authorization token file for KUKSA.val should ideally not
    # be readable by other users, but currently that's not doable
    # until a packaging/sandboxing/MAC scheme is (re)implemented or
    # something like OAuth is plumbed in as an alternative.
    install -d ${D}${sysconfdir}/xdg/AGL/cluster-dashboard
    install -m 0644 ${WORKDIR}/cluster-dashboard.conf.default ${D}${sysconfdir}/xdg/AGL/
    install -m 0644 ${WORKDIR}/cluster-dashboard.conf.demo ${D}${sysconfdir}/xdg/AGL/
    install -m 0644 ${WORKDIR}/cluster-dashboard.token ${D}${sysconfdir}/xdg/AGL/cluster-dashboard/
}

ALTERNATIVE_LINK_NAME[cluster-dashboard.conf] = "${sysconfdir}/xdg/AGL/cluster-dashboard.conf"

PACKAGE_BEFORE_PN += "${PN}-conf"
FILES:${PN}-conf += "${sysconfdir}/xdg/AGL/cluster-dashboard.conf.default"
RDEPENDS:${PN}-conf = "${PN}"
RPROVIDES:${PN}-conf = "cluster-dashboard.conf"
ALTERNATIVE:${PN}-conf = "cluster-dashboard.conf"
ALTERNATIVE_TARGET_${PN}-conf = "${sysconfdir}/xdg/AGL/cluster-dashboard.conf.default"

PACKAGE_BEFORE_PN += "${PN}-conf-demo"
FILES:${PN}-conf-demo += "${sysconfdir}/xdg/AGL/cluster-dashboard.conf.demo"
RDEPENDS:${PN}-conf-demo = "${PN}"
RPROVIDES:${PN}-conf-demo = "cluster-dashboard.conf"
ALTERNATIVE:${PN}-conf-demo = "cluster-dashboard.conf"
ALTERNATIVE_TARGET_${PN}-conf-demo = "${sysconfdir}/xdg/AGL/cluster-dashboard.conf.demo"

# NOTE: Not currently used in KVM demo, so no extra configurations packaged here

RDEPENDS:${PN} += " \
    qtwayland \
    qtbase-qmlplugins \
    qtdeclarative \
    qt5compat \
    qtsvg-plugins \
    qtdeclarative-qmlplugins \
    qtwayland \
    qtlocation \
    qtlocation-qmlplugins \
    qtlocation-plugins \
    qtwebengine \
    qtwebengine-qmlplugins \
    qtwebchannel \
    qtwebchannel-qmlplugins \
    qtquickcontrols2-agl-style \
    sqlite3 \
"
