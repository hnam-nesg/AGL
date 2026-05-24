SUMMARY = "phoBert"
LICENSE = "CLOSED"

SRC_URI = "file://bpe.codes \
           file://multihead_label_maps.json \
           file://phobert_multihead_int8.onnx \
           file://vocab.txt \
        "

S = "${WORKDIR}"

do_install() {
    install -d ${D}${datadir}/assistant/intent
    install -m 0644 ${WORKDIR}/bpe.codes \
        ${D}${datadir}/assistant/intent/bpe.codes
    install -m 0644 ${WORKDIR}/multihead_label_maps.json \
        ${D}${datadir}/assistant/intent/multihead_label_maps.json
    install -m 0644 ${WORKDIR}/phobert_multihead_int8.onnx \
        ${D}${datadir}/assistant/intent/phobert_multihead_int8.onnx
    install -m 0644 ${WORKDIR}/vocab.txt \
        ${D}${datadir}/assistant/intent/vocab.txt
}

FILES:${PN} += "${datadir}/assistant/intent/bpe.codes \
                ${datadir}/assistant/intent/multihead_label_maps.json \
                ${datadir}/assistant/intent/phobert_multihead_int8.onnx \
                ${datadir}/assistant/intent/vocab.txt \
            "