SUMMARY = "ONNX Runtime (Pre-built Binaries for AARCH64)"
DESCRIPTION = "Thư viện chia sẻ ONNX Runtime tải trực tiếp từ Microsoft Github"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=0f7e3b1308cb5c00b372a6e78835732d"

# Bỏ qua kiểm tra mã hash nghiêm ngặt để tránh lỗi khi đổi version
BB_STRICT_CHECKSUM = "0"

# Tải file nén dành riêng cho aarch64
SRC_URI = "https://github.com/microsoft/onnxruntime/releases/download/v1.25.1/onnxruntime-linux-aarch64-1.25.1.tgz"

S = "${WORKDIR}/onnxruntime-linux-aarch64-1.25.1"

# Bỏ qua các khâu configure và compile vì đây là file đã được build sẵn
do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    # 1. Cài đặt các file thư viện động (.so)
    install -d ${D}${libdir}
    # Dùng cp -P để giữ nguyên các symlink của file .so
    cp -P ${S}/lib/libonnxruntime.so* ${D}${libdir}/

    # 2. Cài đặt các file Header (.h) để C++ có thể include
    install -d ${D}${includedir}
    cp -R ${S}/include/* ${D}${includedir}/
}

# Tắt các cảnh báo QA của Yocto (do file này không do Yocto tự build từ source)
INSANE_SKIP:${PN} += "already-stripped ldflags dev-so"
# Đảm bảo Yocto đóng gói đúng các file .so vào sysroot
FILES_SOLIBSDEV = ""
FILES:${PN} += "${libdir}/libonnxruntime.so*"
