SUMMARY = "Piper TTS Engine (GPLv3) and Vietnamese Model for AGL"
DESCRIPTION = "Offline Text-to-Speech engine using VITS and ONNX Runtime (OHF-Voice branch)"
HOMEPAGE = "https://github.com/OHF-Voice/piper1-gpl"
LICENSE = "GPL-3.0-only"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-3.0-only;md5=c79ff39f19dfec6d293b95dea7b07891"

SRC_URI = " \
    https://github.com/rhasspy/piper/releases/download/2023.11.14-2/piper_linux_aarch64.tar.gz;name=piperbin \
    file://vi_VN-vais1000-medium.onnx \
    file://vi_VN-vais1000-medium.onnx.json \
"

# Chỉ giữ lại mã băm cho file .tar.gz tải từ GitHub, xóa các dòng mã băm của file onnx đi
SRC_URI[piperbin.sha256sum] = "fea0fd2d87c54dbc7078d0f878289f404bd4d6eea6e7444a77835d1537ab88eb"

# Bỏ qua kiểm tra mã băm (Trong môi trường thực tế, bạn nên tải file về và chạy sha256sum để điền vào đây cho an toàn)
SRC_URI[piperbin.sha256sum] = "fea0fd2d87c54dbc7078d0f878289f404bd4d6eea6e7444a77835d1537ab88eb"
SRC_URI[modelonnx.sha256sum] = "6ab13374eb0862021a545befe7727aef59e16117f1c075aa9e0362237ecc98ae"
SRC_URI[modeljson.sha256sum] = "9dc373d69b0e4f39d864cb8ac9fe42cee49817a0f02d831e57a9ce6e944e234a"

S = "${WORKDIR}/piper"

# Bỏ qua bước configure và compile vì chúng ta dùng pre-built binary
do_configure[noexec] = "1"
do_compile[noexec] = "1"

# Tắt cảnh báo QA của Yocto về việc binary đã bị strip (loại bỏ symbol debug)
INSANE_SKIP:${PN} += "already-stripped ldflags libdir dev-so"

do_install() {
    # 1. Cài đặt toàn bộ Engine và các thư viện .so đi kèm vào /opt/piper
    install -d ${D}/opt/piper
    cp -r ${S}/* ${D}/opt/piper/
    chmod +x ${D}/opt/piper/piper

    # 2. Tạo symlink ra /usr/bin để hệ thống AGL có thể gọi lệnh 'piper' ở bất kỳ đâu
    install -d ${D}${bindir}
    ln -sf /opt/piper/piper ${D}${bindir}/piper

    # 3. Cài đặt Model Tiếng Việt
    install -d ${D}${datadir}/piper/models
    install -m 0644 ${WORKDIR}/vi_VN-vais1000-medium.onnx ${D}${datadir}/piper/models/
    install -m 0644 ${WORKDIR}/vi_VN-vais1000-medium.onnx.json ${D}${datadir}/piper/models/
}

# Khai báo các file sẽ được đóng gói vào rootfs của image
FILES:${PN} += " \
    /opt/piper/* \
    ${bindir}/piper \
    ${datadir}/piper/models/* \
"
