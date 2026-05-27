import QtQuick

Item {
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
                                                base.addColorStop(0.00, themeSettings.baseColor0); // trái: đen nhất
                                                base.addColorStop(0.35, themeSettings.baseColor1); // vẫn rất đen
                                                base.addColorStop(0.55, themeSettings.baseColor2); // tới giữa: bắt đầu nhạt hơn chút
                                                base.addColorStop(1.00, themeSettings.baseColor3); // phải: vẫn tối
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

                                                glow.addColorStop(0.00, themeSettings.rgba(themeSettings.glowColor0, themeSettings.glowAlpha0));  // xanh navy sáng (không tím)
                                                glow.addColorStop(0.22, themeSettings.rgba(themeSettings.glowColor1, themeSettings.glowAlpha1));  // xanh đậm
                                                glow.addColorStop(0.55, themeSettings.rgba(themeSettings.glowColor2, themeSettings.glowAlpha2));   // xanh rất đậm mờ
                                                glow.addColorStop(1.00, "rgba(0, 0, 0, 0.00)");     // tan vào nền




                                                ctx.fillStyle = glow;
                                                ctx.fillRect(0, 0, w, h);

                                                // 3) Lớp “sweep” nhẹ (giữ rất tối bên trái, chỉ nhấn màu ở phía phải)
                                                let sweep = ctx.createLinearGradient(0, h, w, 0);
                                                sweep.addColorStop(0.00, themeSettings.rgba(themeSettings.sweepColor0, themeSettings.sweepAlpha0));
                                                sweep.addColorStop(0.50, themeSettings.rgba(themeSettings.sweepColor1, themeSettings.sweepAlpha1));
                                                sweep.addColorStop(0.80, themeSettings.rgba(themeSettings.sweepColor2, themeSettings.sweepAlpha2));
                                                sweep.addColorStop(1.00, themeSettings.rgba(themeSettings.sweepColor3, themeSettings.sweepAlpha3));

                                                ctx.fillStyle = sweep;
                                                ctx.fillRect(0, 0, w, h);
                                }

                                // tự vẽ lại khi đổi kích thước
                                onWidthChanged: requestPaint()
                                onHeightChanged: requestPaint()
                                Component.onCompleted: requestPaint()
                }

                Connections {
                                target: themeSettings
                                function onThemeChanged() { bg.requestPaint() }
                }
}

