SUMMARY = "C++ Wrapper cho openWakeWord sử dụng ONNX Runtime"
DESCRIPTION = "Shared library cung cấp Wake Word Engine (Alexa) cho hệ thống AGL"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Phụ thuộc vào onnxruntime (Đảm bảo sysroot đã có thư viện này trước khi CMake chạy)
DEPENDS = "onnxruntime"

# Nguồn file từ thư mục local 'files/'
SRC_URI = " \
    file://CMakeLists.txt \
    file://src/ \
    file://include/ \
    file://models/ \
"

# Do lấy trực tiếp từ thư mục files/, mã nguồn nằm ngay tại WORKDIR
S = "${WORKDIR}"

# Sử dụng CMake để tự động cấu hình và biên dịch
inherit cmake

# Tạo thêm một package phụ chỉ để chứa các file AI Models
PACKAGES =+ "${PN}-models"

# Bổ sung bước cài đặt file ONNX (vì CMakeLists.txt thường không xử lý file model)
do_install:append() {
    install -d ${D}${datadir}/openwakeword/models
    install -m 0644 ${S}/models/*.onnx ${D}${datadir}/openwakeword/models/
}

# =====================================================================
# CẤU HÌNH ĐÓNG GÓI PACKAGES (PACKAGING RULES)
# =====================================================================

# 1. Package Models: Chứa các file nặng, giúp dễ update OTA sau này
FILES:${PN}-models = "${datadir}/openwakeword/models/*.onnx"

# 2. Xử lý triệt để lỗi QA [dev-elf] của Yocto:
# Mặc định Yocto nghĩ file .so là symlink và nhét vào package -dev.
# Lệnh dưới đây tắt quy tắc đó đối với thư viện không có đánh số version.
FILES_SOLIBSDEV = ""

# 3. Package Chính: Chứa file nhị phân thư viện (libwakeword.so) để chạy thực tế
FILES:${PN} += "${libdir}/lib*.so"

# 4. Package Dev: Chứa file Header (.h) để các ứng dụng Qt/C++ khác include vào
FILES:${PN}-dev += "${includedir}/openwakeword/*.h"
