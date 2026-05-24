SUMMARY = "Sherpa-ONNX ASR Engine"
DESCRIPTION = "Next-gen Kaldi speech recognition engine"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=3b83ef96387f14655fc854ddc3c6bd57"

SRCREV_FORMAT = "sherpa"
SRCREV_sherpa = "${AUTOREV}"
SRCREV_fbank = "v1.22.3"
SRCREV_kdecoder = "v0.3.0"
SRCREV_spiece = "v0.7"
SRCREV_json = "v3.12.0"
SRCREV_kissfft = "febd4caeed32e33ad8b2e0bb5ea77542c40f18ec"
SRCREV_kaldifst = "v1.8.0"
SRCREV_eigen = "3147391d946bb4b6c68edd901f2add6ac1f31f8c"
# Thêm tag chuẩn cho openfst dựa theo log của bác
SRCREV_openfst = "v1.8.5-2026-04-11"

SRC_URI = " \
    git://github.com/k2-fsa/sherpa-onnx.git;protocol=https;branch=master;name=sherpa \
    git://github.com/csukuangfj/kaldi-native-fbank.git;protocol=https;nobranch=1;name=fbank;destsuffix=deps/kaldi_native_fbank-src \
    git://github.com/k2-fsa/kaldi-decoder.git;protocol=https;nobranch=1;name=kdecoder;destsuffix=deps/kaldi_decoder-src \
    git://github.com/pkufool/simple-sentencepiece.git;protocol=https;nobranch=1;name=spiece;destsuffix=deps/simple-sentencepiece-src \
    git://github.com/nlohmann/json.git;protocol=https;nobranch=1;name=json;destsuffix=deps/json-src \
    git://github.com/mborgerding/kissfft.git;protocol=https;nobranch=1;name=kissfft;destsuffix=deps/kissfft-src \
    git://github.com/k2-fsa/kaldifst.git;protocol=https;nobranch=1;name=kaldifst;destsuffix=deps/kaldifst-src \
    git://gitlab.com/libeigen/eigen.git;protocol=https;nobranch=1;name=eigen;destsuffix=deps/eigen-src \
    git://github.com/csukuangfj/openfst.git;protocol=https;nobranch=1;name=openfst;destsuffix=deps/openfst-src \
"

S = "${WORKDIR}/git"

DEPENDS = "onnxruntime alsa-lib"

inherit cmake

EXTRA_OECMAKE = " \
    -DBUILD_SHARED_LIBS=ON \
    -DSHERPA_ONNX_ENABLE_PYTHON=OFF \
    -DSHERPA_ONNX_ENABLE_TESTS=OFF \
    -DSHERPA_ONNX_ENABLE_C_API=ON \
    -DSHERPA_ONNX_ENABLE_PORTAUDIO=OFF \
    -DSHERPA_ONNX_ENABLE_WEBSOCKET=OFF \
    -DSHERPA_ONNX_ENABLE_BINARY=OFF \
    -DSHERPA_ONNX_ENABLE_TTS=OFF \
    -DSHERPA_ONNX_ENABLE_SPEAKER_DIARIZATION=OFF \
"

do_configure:prepend() {
    bbnote "Pre-populating CMake _deps folder to bypass network downloads..."
    mkdir -p ${B}/_deps
    
    cp -r ${WORKDIR}/deps/kaldi_native_fbank-src ${B}/_deps/
    cp -r ${WORKDIR}/deps/kaldi_decoder-src ${B}/_deps/
    cp -r ${WORKDIR}/deps/simple-sentencepiece-src ${B}/_deps/
    cp -r ${WORKDIR}/deps/json-src ${B}/_deps/
    cp -r ${WORKDIR}/deps/kissfft-src ${B}/_deps/
    cp -r ${WORKDIR}/deps/kaldifst-src ${B}/_deps/
    cp -r ${WORKDIR}/deps/eigen-src ${B}/_deps/
    # Bơm nốt openfst vào cho kaldifst nó gọi
    cp -r ${WORKDIR}/deps/openfst-src ${B}/_deps/
}

do_install:append() {
    # 1. Tạo thư mục pkgconfig chuẩn (/usr/lib/pkgconfig)
    install -d ${D}${libdir}/pkgconfig
    
    # 2. Dời file mồ côi vào đúng vị trí
    if [ -f ${D}${prefix}/sherpa-onnx.pc ]; then
        mv ${D}${prefix}/sherpa-onnx.pc ${D}${libdir}/pkgconfig/
    fi
}

FILES_SOLIBSDEV = ""
SOLIBS = ".so*"

FILES:${PN} += "${libdir}/lib*.so*"
FILES:${PN}-dev += "${includedir}/sherpa-onnx/*"
ALLOW_EMPTY:${PN} = "1"

PROVIDES = "sherpa-onnx"
INSANE_SKIP:${PN} += "dev-so"