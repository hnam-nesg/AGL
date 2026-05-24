import QtQuick
import QtQuick.Effects
Item {
    id: root
    height: parent.height
    width: parent.width
    //anchors.centerIn: parent
    property real radius: 10
    property real borderWidth: 1
    property color c1: "#3f6f6f6f"
    property color c2: "#3f6f6f6f"//"#202a36"

    Canvas {
        id: borderCanvas
        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d");
            ctx.reset();

            const w = width, h = height, bw = borderWidth;
            const r = Math.max(0, radius - bw/2);

            const grad = ctx.createLinearGradient(0, 0, w, 0);
            grad.addColorStop(0, c1);
            grad.addColorStop(1, c2);

            ctx.lineWidth = bw;
            ctx.strokeStyle = grad;
            ctx.lineCap = "round";
            ctx.lineJoin = "round";

            const inset = bw/2;
            const x = inset, y = inset, ww = w - bw, hh = h - bw;

            ctx.beginPath();
            ctx.moveTo(x + r, y);
            ctx.lineTo(x + ww - r, y);
            ctx.arcTo(x + ww, y, x + ww, y + r, r);
            ctx.lineTo(x + ww, y + hh - r);
            ctx.arcTo(x + ww, y + hh, x + ww - r, y + hh, r);
            ctx.lineTo(x + r, y + hh);
            ctx.arcTo(x, y + hh, x, y + hh - r, r);
            ctx.lineTo(x, y + r);
            ctx.arcTo(x, y, x + r, y, r);
            ctx.stroke();
        }

        Connections {
            target: root
            function onC1Changed(){ borderCanvas.requestPaint() }
            function onC2Changed(){ borderCanvas.requestPaint() }
        }
    }
    // SequentialAnimation on c1 {
    //     loops: Animation.Infinite
    //     ColorAnimation { to: root.c2; duration: 1200; easing.type: Easing.InOutSine }
    //     ColorAnimation { to: root.c1; duration: 1200; easing.type: Easing.InOutSine }
    // }
    // SequentialAnimation on c2 {
    //     loops: Animation.Infinite
    //     ColorAnimation { to: root.c1; duration: 1200; easing.type: Easing.InOutSine }
    //     ColorAnimation { to: root.c2; duration: 1200; easing.type: Easing.InOutSine }
    // }
}
