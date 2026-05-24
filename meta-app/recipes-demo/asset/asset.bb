DESCRIPTION = "Bulk assets package"
LICENSE = "CLOSED"

SRC_URI += "file://asset/"

S = "${WORKDIR}"

do_install() {
    install -d ${D}/usr/bin/images
    cp -r ${WORKDIR}/asset/* ${D}/usr/bin/images/
}

FILES:${PN} += "/usr/bin/images"
