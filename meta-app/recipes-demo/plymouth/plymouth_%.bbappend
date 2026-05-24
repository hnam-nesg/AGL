FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://mytheme.plymouth \
    file://mytheme.script \
    file://logo.png \
    file://plymouthd.conf \
    file://Panel.png \
    file://Panel_2.png \
"

PLYMOUTH_THEMES = "script"
PACKAGECONFIG = "drm udev ${PLYMOUTH_THEMES} ${@bb.utils.filter('DISTRO_FEATURES', 'systemd', d)}"

do_install:append () {
    install -d ${D}${datadir}/plymouth/themes/mytheme

    install -m 0644 ${WORKDIR}/mytheme.plymouth \
        ${D}${datadir}/plymouth/themes/mytheme/mytheme.plymouth

    install -m 0644 ${WORKDIR}/mytheme.script \
        ${D}${datadir}/plymouth/themes/mytheme/mytheme.script

    install -m 0644 ${WORKDIR}/logo.png \
        ${D}${datadir}/plymouth/themes/mytheme/logo.png
    
    install -m 0644 ${WORKDIR}/Panel.png \
        ${D}${datadir}/plymouth/themes/mytheme/Panel.png

    install -m 0644 ${WORKDIR}/Panel_2.png \
        ${D}${datadir}/plymouth/themes/mytheme/Panel_2.png

    install -d ${D}${sysconfdir}/plymouth
    install -m 0644 ${WORKDIR}/plymouthd.conf ${D}${sysconfdir}/plymouth/plymouthd.conf
}
