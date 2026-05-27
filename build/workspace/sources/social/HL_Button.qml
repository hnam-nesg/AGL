import QtQuick
import Qt5Compat.GraphicalEffects

Rectangle{
    Canvas {
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d");
            ctx.reset();

            const r = 15; // radius (px) - chỉnh theo ý bạn

            // Gradient ngang: trái -> phải
            const g = ctx.createLinearGradient(0, 0, width, 0);
            g.addColorStop(0.3, "black");
            g.addColorStop(1.0, "#fa001b53");

            // Rounded-rect path
            const x = 0, y = 0, w = width, h = height;
            const rr = Math.max(0, Math.min(r, Math.min(w, h) / 2));

            ctx.beginPath();
            ctx.moveTo(x + rr, y);
            ctx.lineTo(x + w - rr, y);
            ctx.quadraticCurveTo(x + w, y, x + w, y + rr);
            ctx.lineTo(x + w, y + h - rr);
            ctx.quadraticCurveTo(x + w, y + h, x + w - rr, y + h);
            ctx.lineTo(x + rr, y + h);
            ctx.quadraticCurveTo(x, y + h, x, y + h - rr);
            ctx.lineTo(x, y + rr);
            ctx.quadraticCurveTo(x, y, x + rr, y);
            ctx.closePath();

            ctx.fillStyle = g;
            ctx.fill();
        }
    }
    layer.enabled: true
    layer.effect: DropShadow {
        horizontalOffset: 0; verticalOffset: 0; radius: 20
        samples: 20; color: "#80FFFFFF"
    }
}
