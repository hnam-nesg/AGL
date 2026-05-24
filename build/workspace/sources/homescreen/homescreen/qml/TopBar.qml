import QtQuick
import QtQuick.Effects

Item {
    id: topbar
    width: parent.width
    height: 50

    FontLoader{
        id: fontLoader
        source: "images/Roboto-BoldCondensed.ttf"
    }

    Canvas {
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d");
            ctx.reset();

            const g = ctx.createLinearGradient(0, 0, width, 0);
            g.addColorStop(0.5, "#4e000000");
            g.addColorStop(1.0, "#2a000000");

            ctx.fillStyle = g;
            ctx.fillRect(0, 0, width, height);
        }
    }

    Canvas {
        id: c
        anchors.fill: parent
        antialiasing: true
        z: 1

        // ===== STYLE =====
        property color strokeColor: "white"
        property real lineW: 1

        // ===== SHAPE =====
        property real pad: 50
        property real yBase: 3         // y ở ngoài biên
        property real gap: 40                      // khoảng hở giữa (để logo)
        property real centerDip: 50                // độ "tụt xuống" gần giữa
        property real curveTight: 0             // độ gắt cong gần giữa (0.3..0.8)

        // ===== FADE (mờ dần từ giữa ra biên) =====
        property real alphaCenter: 1.0
        property real alphaEdge: 0

        function rgbaStr(a) {
            // Canvas muốn rgba(...) dạng string
            const r = Math.round(strokeColor.r * 255)
            const g = Math.round(strokeColor.g * 255)
            const b = Math.round(strokeColor.b * 255)
            return "rgba(" + r + "," + g + "," + b + "," + a + ")"
        }

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()

            const dpr = Screen.devicePixelRatio || 1
            ctx.scale(dpr, dpr)

            const w = width
            const cx = w / 2

            const leftEndX   = cx - gap / 2
            const rightStartX = cx + gap / 2

            const yOuter  = yBase
            const yCenter = yBase + centerDip   // tụt xuống ở giữa

            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            // ---- helper: vẽ 1 nửa với gradient alpha ----
            function strokeHalf(isLeft) {
                ctx.beginPath()

                if (isLeft) {
                    // Gradient: ngoài biên nhạt -> gần giữa đậm
                    const gL = ctx.createLinearGradient(pad, 0, leftEndX, 0)
                    gL.addColorStop(0.0, rgbaStr(alphaEdge))
                    gL.addColorStop(1.0, rgbaStr(alphaCenter))
                    ctx.strokeStyle = gL

                    // Path: từ trái ngoài biên -> cong xuống và kết ở gần giữa
                    ctx.moveTo(pad, yOuter)

                    // cubic cho "đuôi" cong mềm rồi tụt xuống
                    const x1 = pad + (leftEndX - pad) * 1
                    const x2 = pad + (leftEndX - pad) * 0.92
                    ctx.bezierCurveTo(
                                x1, yOuter,                          // cp1 giữ ổn ngoài biên
                                x2, yOuter + centerDip * curveTight, // cp2 kéo xuống gần giữa
                                leftEndX, yCenter                    // end tụt xuống
                                )
                } else {
                    // Gradient: gần giữa đậm -> ra ngoài biên nhạt
                    const gR = ctx.createLinearGradient(rightStartX, 0, w - pad, 0)
                    gR.addColorStop(0.0, rgbaStr(alphaCenter))
                    gR.addColorStop(1.0, rgbaStr(alphaEdge))
                    ctx.strokeStyle = gR

                    ctx.moveTo(rightStartX, yCenter)

                    const x1 = rightStartX + (w - pad - rightStartX) * 0.08
                    const x2 = rightStartX + (w - pad - rightStartX) * 0
                    ctx.bezierCurveTo(
                                x1, yOuter + centerDip * curveTight, // cp1 kéo xuống gần giữa
                                x2, yOuter,                          // cp2 trở lại phẳng dần
                                w - pad, yOuter
                                )
                }

                // (Tuỳ chọn) vẽ glow mỏng phía ngoài (giống line có halo)
                ctx.lineWidth = lineW + 2.8
                const oldStroke = ctx.strokeStyle
                // giảm alpha glow
                // Với gradient, ta vẽ thêm 1 lần với globalAlpha
                ctx.save()
                ctx.globalAlpha = 0.25
                ctx.stroke()
                ctx.restore()

                // vẽ line chính
                ctx.lineWidth = lineW
                ctx.strokeStyle = oldStroke
                ctx.stroke()
            }

            strokeHalf(true)
            strokeHalf(false)
        }

        function repaint() { requestPaint() }
        Component.onCompleted: repaint()
        onWidthChanged: repaint()
        onHeightChanged: repaint()
        onStrokeColorChanged: repaint()
        onLineWChanged: repaint()
        onPadChanged: repaint()
        onYBaseChanged: repaint()
        onGapChanged: repaint()
        onCenterDipChanged: repaint()
        onCurveTightChanged: repaint()
        onAlphaCenterChanged: repaint()
        onAlphaEdgeChanged: repaint()
    }

    MultiEffect {
        id: effect_text
        anchors.fill: c
        source: c
        shadowEnabled: true
        shadowColor: "white"
        shadowOpacity: 1
        shadowBlur: 0.5
        shadowHorizontalOffset: 2
        shadowVerticalOffset: 2
    }

    Image{
        id: logo
        width: 55; height: 55
        anchors.centerIn: parent
        source: "images/mercedes.png"
        MultiEffect {
            anchors.fill: parent
            source: parent
            shadowEnabled: true
            shadowColor: "black"
            shadowOpacity: 1
            shadowBlur: 0.5
            shadowHorizontalOffset: 1
            shadowVerticalOffset: 1
        }
    }

    property var vi: Qt.locale("vi_VN")

    Timer {
        id: timer
        interval: 1000; running: true; repeat: true
        onTriggered: {
            day_name.text = Qt.formatTime(new Date(), "HH:mm AP")
        }
    }

    Text {
        id: day_name
        x: 905; y: 10; width: 50; height: 30; font.pixelSize: 20
        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.family: fontLoader.name; color: "white"
    }
    MultiEffect {
        anchors.fill: day_name; source: day_name;  shadowEnabled: true; shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.5; shadowHorizontalOffset: 5; shadowVerticalOffset: 5
    }

    Image{
        width: 25; height: 30; x: 1235; y: 10; source: "images/airbag.svg"; fillMode: Image.PreserveAspectCrop
    }

    Text{
        width: 50; height: 30; x: 1155; y: 10; font.family: fontLoader.name; horizontalAlignment: Text.AlignHLeft; verticalAlignment: Text.AlignVCenter
        text: "PASSENGER\nAIR BAG OFF"; font.pixelSize: 15; color: "#f9c200"
    }

    ButtonControl{
        x: 1105; y: 10; width: 30; height: 30; source: "images/mobile_phone.png"; width_image: 30; height_image: 30
    }
    ButtonControl{
        x: 1025; y: 10; width: 60; height: 30; source__: "images/account.png"; activeText: true; contentText: "Mercedes"; fontSize: 18
    }

}

