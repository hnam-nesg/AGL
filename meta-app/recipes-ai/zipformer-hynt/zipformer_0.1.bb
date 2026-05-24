SUMMARY = "Zipformer 30M RNNT Vietnamese Model by hynt"
DESCRIPTION = "Pre-trained Vietnamese ASR model optimized for Sherpa-ONNX"
LICENSE = "MIT"
# Vì model thường không có file LICENSE đi kèm, ta lấy đại file README làm mốc check
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Bác nén thư mục model trên WSL thành file .tar.gz để Yocto dễ xử lý
# Hoặc trỏ thẳng vào link Hugging Face nếu bác muốn Yocto tự tải (nhưng nên dùng file local cho nhanh)
SRC_URI = "file://zipformer-hynt.tar.gz"

S = "${WORKDIR}"

do_install() {
    # Tạo thư mục đích trên target
    install -d ${D}/usr/share/zipformer-hynt/models
    
    # Copy toàn bộ file ONNX và tokens vào
    install -m 0644 ${S}/zipformer-hynt/*.onnx ${D}/usr/share/zipformer-hynt/models/
    install -m 0644 ${S}/zipformer-hynt/tokens.txt ${D}/usr/share/zipformer-hynt/models/
}

# Khai báo để Yocto không quét lỗi "file nằm ngoài thư mục chuẩn"
FILES:${PN} += "/usr/share/zipformer-hynt/models/*"

# Không cần compile, chỉ là dữ liệu thô
INHIBIT_DEFAULT_DEPS = "1"
