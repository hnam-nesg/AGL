SUMMARY = "llama.cpp local inference engine and library"
DESCRIPTION = "Build llama.cpp shared library and optional server"
HOMEPAGE = "https://github.com/ggml-org/llama.cpp"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=223b26b3c1143120c87e2b13111d3e99"

PV = "git${SRCPV}"
SRCREV = "${AUTOREV}"

SRC_URI = "git://github.com/ggml-org/llama.cpp.git;branch=master;protocol=https \
           file://llama-server.service \
           file://llama-server.env \
"

S = "${WORKDIR}/git"

inherit cmake pkgconfig systemd

DEPENDS = "zlib"

# Lưu ý nhỏ: Việc thêm RDEPENDS này sẽ bắt buộc cài whisper-cpp khi cài llama.cpp.
# Nếu bạn đã thêm cả hai vào IMAGE_INSTALL trong file cấu hình image AGL thì dòng này có thể bỏ qua, 
# nhưng để nguyên cũng không ảnh hưởng gì.

SYSTEMD_PACKAGES = "${PN}-server"
SYSTEMD_SERVICE:${PN}-server = "llama-server.service"
SYSTEMD_AUTO_ENABLE:${PN}-server = "enable"

DEPENDS += " \
    vulkan-loader \
    vulkan-headers \
    shaderc-native glslang-native \
    spirv-headers \
"
RDEPENDS:${PN} += "vulkan-loader mesa-vulkan-drivers"

# Đổi thư mục cài đặt sang /usr/lib/llama và cấu hình RPATH
EXTRA_OECMAKE += "\
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_INSTALL_LIBDIR=lib/llama \
    -DCMAKE_INSTALL_INCLUDEDIR=include/llama \
    -DCMAKE_INSTALL_RPATH=${libdir}/llama \
    -DLLAMA_BUILD_COMMON=ON \
    -DLLAMA_BUILD_TESTS=OFF \
    -DLLAMA_BUILD_EXAMPLES=OFF \
    -DLLAMA_BUILD_TOOLS=ON \
    -DLLAMA_BUILD_SERVER=ON \
    -DLLAMA_BUILD_WEBUI=OFF \
    -DLLAMA_OPENSSL=OFF \
    -DGGML_BLAS=OFF \
    -DGGML_CUDA=OFF \
    -DGGML_METAL=OFF \
    -DGGML_VULKAN=OFF \
    -DGGML_OPENMP=ON \
"

do_install:append() {
    install -d ${D}${bindir}
    install -d ${D}${systemd_system_unitdir}
    install -d ${D}${sysconfdir}/default
    install -d ${D}${datadir}/llama.cpp/models

    if [ -f ${B}/bin/llama-server ]; then
        install -m 0755 ${B}/bin/llama-server ${D}${bindir}/llama-server
    elif [ -f ${B}/llama-server ]; then
        install -m 0755 ${B}/llama-server ${D}${bindir}/llama-server
    fi

    if [ -f ${B}/bin/llama-cli ]; then
        install -m 0755 ${B}/bin/llama-cli ${D}${bindir}/llama-cli || true
    elif [ -f ${B}/llama-cli ]; then
        install -m 0755 ${B}/llama-cli ${D}${bindir}/llama-cli || true
    fi

    install -m 0644 ${WORKDIR}/llama-server.service ${D}${systemd_system_unitdir}/llama-server.service
    install -m 0644 ${WORKDIR}/llama-server.env ${D}${sysconfdir}/default/llama-server
}

# Tách riêng gói ggml
PACKAGES =+ "${PN}-server ${PN}-cli ${PN}-ggml"

FILES:${PN}-server += "\
    ${bindir}/llama-server \
    ${systemd_system_unitdir}/llama-server.service \
    ${sysconfdir}/default/llama-server \
"

FILES:${PN}-cli += "\
    ${bindir}/llama-cli \
"

# Đóng gói các thư viện chia sẻ (.so.*) của ggml
FILES:${PN}-ggml += "\
    ${libdir}/llama/libggml*.so.* \
"

# Khai báo libllama và bổ sung thêm thư viện libmtmd vào gói chính
FILES:${PN} += "\
    ${libdir}/llama/libllama*.so.* \
    ${libdir}/llama/libmtmd*.so.* \
"

# Bổ sung thư mục pkgconfig vào gói -dev (dành cho lập trình/biên dịch App Qt)
FILES:${PN}-dev += "\
    ${includedir}/* \
    ${libdir}/llama/*.so \
    ${libdir}/llama/cmake/* \
    ${libdir}/llama/pkgconfig/* \
"

INSANE_SKIP:${PN} += "dev-so"
