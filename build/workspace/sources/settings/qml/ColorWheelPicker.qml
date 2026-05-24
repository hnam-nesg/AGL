// ColorWheelPicker.qml
import QtQuick

Item {
    id: picker

    width: 340
    height: 340

    property real hue: 0.0
    property real saturation: 1.0
    property real brightness: 1.0

    readonly property real cx: width / 2
    readonly property real cy: height / 2
    readonly property real outerRadius: Math.min(width, height) * 0.46
    readonly property real ringWidth: 34
    readonly property real ringInnerRadius: outerRadius - ringWidth
    readonly property real ringMidRadius: outerRadius - ringWidth / 2
    readonly property real discRadius: ringInnerRadius - 18

    readonly property color selectedColor: Qt.hsva(hue, saturation, brightness, 1.0)
    readonly property string selectedHex: rgbToHex(hsvToRgb(hue, saturation, brightness))

    signal edited(string hexColor)
    signal touchStarted()
    signal touchEnded()

    onHueChanged: {
        colorCanvas.requestPaint()
        edited(selectedHex)
    }

    onSaturationChanged: edited(selectedHex)
    onBrightnessChanged: edited(selectedHex)

    function clamp(v, minV, maxV) {
        return Math.max(minV, Math.min(maxV, v))
    }

    function hsvToRgb(h, s, v) {
        h = ((h % 1.0) + 1.0) % 1.0
        s = clamp(s, 0.0, 1.0)
        v = clamp(v, 0.0, 1.0)

        var r = 0
        var g = 0
        var b = 0

        var i = Math.floor(h * 6.0)
        var f = h * 6.0 - i
        var p = v * (1.0 - s)
        var q = v * (1.0 - f * s)
        var t = v * (1.0 - (1.0 - f) * s)

        switch (i % 6) {
        case 0:
            r = v; g = t; b = p
            break
        case 1:
            r = q; g = v; b = p
            break
        case 2:
            r = p; g = v; b = t
            break
        case 3:
            r = p; g = q; b = v
            break
        case 4:
            r = t; g = p; b = v
            break
        case 5:
            r = v; g = p; b = q
            break
        }

        return {
            r: Math.round(r * 255),
            g: Math.round(g * 255),
            b: Math.round(b * 255)
        }
    }

    function rgbToHex(rgb) {
        function h2(v) {
            var s = Number(v).toString(16).toUpperCase()
            return s.length === 1 ? "0" + s : s
        }

        return "#" + h2(rgb.r) + h2(rgb.g) + h2(rgb.b)
    }

    function hsvCss(h, s, v) {
        var rgb = hsvToRgb(h, s, v)
        return "rgb(" + rgb.r + "," + rgb.g + "," + rgb.b + ")"
    }

    function setFromHex(hex) {
        var t = String(hex).replace("#", "")
        if (t.length !== 6)
            return

        var r = parseInt(t.substring(0, 2), 16) / 255.0
        var g = parseInt(t.substring(2, 4), 16) / 255.0
        var b = parseInt(t.substring(4, 6), 16) / 255.0

        var maxV = Math.max(r, g, b)
        var minV = Math.min(r, g, b)
        var d = maxV - minV

        var h = 0.0
        var s = maxV === 0.0 ? 0.0 : d / maxV
        var v = maxV

        if (d !== 0.0) {
            if (maxV === r)
                h = ((g - b) / d + (g < b ? 6.0 : 0.0)) / 6.0
            else if (maxV === g)
                h = ((b - r) / d + 2.0) / 6.0
            else
                h = ((r - g) / d + 4.0) / 6.0
        }

        hue = h
        saturation = s
        brightness = v
        colorCanvas.requestPaint()
    }

    Canvas {
        id: colorCanvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            // Outer hue wheel
            ctx.lineWidth = picker.ringWidth
            for (var deg = 0; deg < 360; deg += 2) {
                var a1 = deg * Math.PI / 180.0
                var a2 = (deg + 2.5) * Math.PI / 180.0

                ctx.beginPath()
                ctx.arc(picker.cx, picker.cy, picker.ringMidRadius, a1, a2, false)
                ctx.strokeStyle = picker.hsvCss(deg / 360.0, 1.0, 1.0)
                ctx.stroke()
            }

            // Inner saturation / brightness disc
            var d = Math.floor(picker.discRadius * 2)
            var img = ctx.createImageData(d, d)

            for (var y = 0; y < d; ++y) {
                for (var x = 0; x < d; ++x) {
                    var dx = x - picker.discRadius
                    var dy = y - picker.discRadius
                    var dist = Math.sqrt(dx * dx + dy * dy)
                    var offset = (y * d + x) * 4

                    if (dist <= picker.discRadius) {
                        var s = picker.clamp(x / d, 0.0, 1.0)
                        var v = picker.clamp(1.0 - y / d, 0.0, 1.0)
                        var rgb = picker.hsvToRgb(picker.hue, s, v)

                        img.data[offset + 0] = rgb.r
                        img.data[offset + 1] = rgb.g
                        img.data[offset + 2] = rgb.b
                        img.data[offset + 3] = 255
                    } else {
                        img.data[offset + 3] = 0
                    }
                }
            }

            ctx.putImageData(img, picker.cx - picker.discRadius, picker.cy - picker.discRadius)

            // Disc border
            ctx.beginPath()
            ctx.arc(picker.cx, picker.cy, picker.discRadius, 0, Math.PI * 2, false)
            ctx.lineWidth = 1
            ctx.strokeStyle = "rgba(255,255,255,0.22)"
            ctx.stroke()
        }
    }

    // Hue handle
    Rectangle {
        width: 18
        height: 18
        radius: 9
        color: "transparent"
        border.width: 3
        border.color: "white"

        x: picker.cx + Math.cos(picker.hue * Math.PI * 2.0) * picker.ringMidRadius - width / 2
        y: picker.cy + Math.sin(picker.hue * Math.PI * 2.0) * picker.ringMidRadius - height / 2

        Rectangle {
            anchors.centerIn: parent
            width: 8
            height: 8
            radius: 4
            color: picker.hsvCss(picker.hue, 1.0, 1.0)
        }
    }

    // Saturation / brightness handle
    Rectangle {
        width: 18
        height: 18
        radius: 9
        color: "transparent"
        border.width: 3
        border.color: "white"

        x: picker.cx - picker.discRadius + picker.saturation * picker.discRadius * 2.0 - width / 2
        y: picker.cy - picker.discRadius + (1.0 - picker.brightness) * picker.discRadius * 2.0 - height / 2

        Rectangle {
            anchors.centerIn: parent
            width: 8
            height: 8
            radius: 4
            color: picker.selectedColor
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true

        function handlePoint(px, py) {
            var dx = px - picker.cx
            var dy = py - picker.cy
            var dist = Math.sqrt(dx * dx + dy * dy)

            if (dist >= picker.ringInnerRadius && dist <= picker.outerRadius) {
                var angle = Math.atan2(dy, dx)
                if (angle < 0)
                    angle += Math.PI * 2.0

                picker.hue = angle / (Math.PI * 2.0)
                colorCanvas.requestPaint()
                return
            }

            if (dist <= picker.discRadius) {
                picker.saturation = picker.clamp(
                    (px - (picker.cx - picker.discRadius)) / (picker.discRadius * 2.0),
                    0.0,
                    1.0
                )

                picker.brightness = picker.clamp(
                    1.0 - (py - (picker.cy - picker.discRadius)) / (picker.discRadius * 2.0),
                    0.0,
                    1.0
                )
            }
        }

        onPressed: function(mouse) {
            picker.touchStarted()
            handlePoint(mouse.x, mouse.y)
        }

        onPositionChanged: function(mouse) {
            if (pressed)
                handlePoint(mouse.x, mouse.y)
        }

        onReleased: {
            picker.touchEnded()
        }

        onCanceled: {
            picker.touchEnded()
        }
    }

    Component.onCompleted: colorCanvas.requestPaint()
}
