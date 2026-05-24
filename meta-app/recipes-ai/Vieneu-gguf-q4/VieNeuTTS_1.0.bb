SUMMARY = "VieNeu TTS model assets"
LICENSE = "CLOSED"

SRC_URI = "file://VieNeu-TTS-0_3B-Q4_0.gguf \
           file://voices.json \
           file://model.onnx \
           file://phoneme_dict.json \
        "

S = "${WORKDIR}"

do_install() {
    install -d ${D}/opt/vieneu-gguf/models/vieneu-q4
    install -d ${D}/opt/vieneu-gguf/models/neucodec
    install -m 0644 ${WORKDIR}/VieNeu-TTS-0_3B-Q4_0.gguf \
        ${D}/opt/vieneu-gguf/models/vieneu-q4/VieNeu-TTS-0_3B-Q4_0.gguf
    install -m 0644 ${WORKDIR}/voices.json \
        ${D}/opt/vieneu-gguf/models/vieneu-q4/voices.json
    install -m 0644 ${WORKDIR}/phoneme_dict.json \
        ${D}/opt/vieneu-gguf/models/vieneu-q4/phoneme_dict.json
    install -m 0644 ${WORKDIR}/model.onnx \
        ${D}/opt/vieneu-gguf/models/neucodec/model.onnx
}

FILES:${PN} += "/opt/vieneu-gguf/models/vieneu-q4/VieNeu-TTS-0_3B-Q4_0.gguf \
                /opt/vieneu-gguf/models/vieneu-q4/voices.json \
                /opt/vieneu-gguf/models/vieneu-q4/phoneme_dict.json \
                /opt/vieneu-gguf/models/neucodec/model.onnx \
            "
