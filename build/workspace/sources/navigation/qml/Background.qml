import QtQuick

Item {
                property int width_: 0
                property int height_: 0
                width: width_; height: height_
                Canvas {
                                id: bg
                                anchors.fill: parent
                                antialiasing: true

                                onPaint: {
                                                const ctx = getContext("2d");
                                                const w = width;
                                                const h = height;

                                                ctx.clearRect(0, 0, w, h);

                                                // 1) Nền tối: rất đen bên trái -> đỡ đen dần tới giữa -> vẫn tối bên phải
                                                let base = ctx.createLinearGradient(0, 0, w, 0);
                                                base.addColorStop(0.00, "#000000"); // trái: đen nhất
                                                base.addColorStop(0.35, "#020202"); // vẫn rất đen
                                                base.addColorStop(0.55, "#080808"); // tới giữa: bắt đầu nhạt hơn chút
                                                base.addColorStop(1.00, "#0b0b0b"); // phải: vẫn tối
                                                ctx.fillStyle = base;
                                                ctx.fillRect(0, 0, w, h);

                                                // 2) Glow cam/vàng: dồn sang phải để trái->giữa vẫn đen nhiều
                                                let glow = ctx.createRadialGradient(
                                                                    w * 0.95, h * 0.45, 10,                 // tâm glow (dịch sang phải)
                                                                    w * 0.95, h * 0.45, Math.max(w, h) * 0.70);

                                                // glow.addColorStop(0.00, "rgba(255, 215, 90, 0.95)"); // vàng
                                                // glow.addColorStop(0.22, "rgba(255, 140, 0, 0.70)");  // cam
                                                // glow.addColorStop(0.55, "rgba(255, 110, 0, 0.22)");  // cam đậm mờ
                                                // glow.addColorStop(1.00, "rgba(0, 0, 0, 0.00)");      // tan vào nền

                                                glow.addColorStop(0.00, "rgba(0, 85, 170, 0.95)");  // xanh navy sáng (không tím)
                                                glow.addColorStop(0.22, "rgba(0, 55, 120, 0.70)");  // xanh đậm
                                                glow.addColorStop(0.55, "rgba(0, 28, 70, 0.22)");   // xanh rất đậm mờ
                                                glow.addColorStop(1.00, "rgba(0, 0, 0, 0.00)");     // tan vào nền




                                                ctx.fillStyle = glow;
                                                ctx.fillRect(0, 0, w, h);

                                                // 3) Lớp “sweep” nhẹ (giữ rất tối bên trái, chỉ nhấn màu ở phía phải)
                                                let sweep = ctx.createLinearGradient(0, h, w, 0);
                                                sweep.addColorStop(0.00, "rgba(0, 0, 0, 0.75)");
                                                sweep.addColorStop(0.50, "rgba(0, 0, 0, 0.35)");
                                                sweep.addColorStop(0.80, "rgba(255, 120, 0, 0.18)");
                                                sweep.addColorStop(1.00, "rgba(255, 210, 80, 0.16)");

                                                ctx.fillStyle = sweep;
                                                ctx.fillRect(0, 0, w, h);
                                }

                                // tự vẽ lại khi đổi kích thước
                                onWidthChanged: requestPaint()
                                onHeightChanged: requestPaint()
                                Component.onCompleted: requestPaint()
                }
}

