SUMMARY = "ConnMan provisioning for static Ethernet IP"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

SRC_URI = "file://my-eth.config"

do_install() {
    install -d ${D}/var/lib/connman
    install -m 0644 ${WORKDIR}/my-eth.config \
        ${D}/var/lib/connman/my-eth.config
}

FILES:${PN} += "/var/lib/connman/my-eth.config"
