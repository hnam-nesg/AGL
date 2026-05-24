// NeonColorWheelPicker.qml
import QtQuick
import QtQuick.Effects

Item {
    id: picker

    width: 330
    height: 330

    property real hue: 0.0
    property real saturation: 0.75
    property real brightness: 0.85

    readonly property real safePadding: Math.min(width, height) * 0.085
    readonly property real usableSize: Math.min(width, height) - safePadding * 2.0

    readonly property real cx: width / 2
    readonly property real cy: height / 2

    readonly property real outerRadius: Math.min(width, height) * 0.47
    readonly property real ringWidth: Math.min(width, height) * 0.078
    readonly property real ringInnerRadius: outerRadius - ringWidth
    readonly property real ringMidRadius: outerRadius - ringWidth / 2
    readonly property real discRadius: ringInnerRadius - Math.min(width, height) * 0.055

    readonly property color selectedColor: Qt.hsva(hue, saturation, brightness, 1.0)
    readonly property string selectedHex: rgbToHex(hsvToRgb(hue, saturation, brightness))

    readonly property real handleSize: usableSize * 0.095
    readonly property real svHandleSize: usableSize * 0.09

    signal edited(string hexColor)
    signal touchStarted()
    signal touchEnded()

    onHueChanged: {
        wheelCanvas.requestPaint()
        glowCanvas.requestPaint()
        edited(selectedHex)
    }

    onSaturationChanged: {
        edited(selectedHex)
    }

    onBrightnessChanged: {
        edited(selectedHex)
    }

    function clamp(v, minValue, maxValue) {
        return Math.max(minValue, Math.min(maxValue, v))
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
            r = v
            g = t
            b = p
            break
        case 1:
            r = q
            g = v
            b = p
            break
        case 2:
            r = p
            g = v
            b = t
            break
        case 3:
            r = p
            g = q
            b = v
            break
        case 4:
            r = t
            g = p
            b = v
            break
        case 5:
            r = v
            g = p
            b = q
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

    function hsvCss(h, s, v, a) {
        var rgb = hsvToRgb(h, s, v)
        if (a === undefined)
            a = 1.0

        return "rgba(" + rgb.r + "," + rgb.g + "," + rgb.b + "," + a + ")"
    }

    function setFromHex(hex) {
        var text = String(hex).replace("#", "")
        if (text.length !== 6)
            return

        var r = parseInt(text.substring(0, 2), 16) / 255.0
        var g = parseInt(text.substring(2, 4), 16) / 255.0
        var b = parseInt(text.substring(4, 6), 16) / 255.0

        if (isNaN(r) || isNaN(g) || isNaN(b))
            return

        var maxValue = Math.max(r, g, b)
        var minValue = Math.min(r, g, b)
        var delta = maxValue - minValue

        var h = 0.0
        var s = maxValue === 0.0 ? 0.0 : delta / maxValue
        var v = maxValue

        if (delta !== 0.0) {
            if (maxValue === r)
                h = ((g - b) / delta + (g < b ? 6.0 : 0.0)) / 6.0
            else if (maxValue === g)
                h = ((b - r) / delta + 2.0) / 6.0
            else
                h = ((r - g) / delta + 4.0) / 6.0
        }

        hue = h
        saturation = s
        brightness = v

        wheelCanvas.requestPaint()
        glowCanvas.requestPaint()
    }

    Rectangle {
        id: backShadow
        anchors.centerIn: parent
        width: parent.width * 0.94
        height: parent.height * 0.94
        radius: width / 2
        color: Qt.rgba(0.02, 0.03, 0.06, 0.88)

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowBlur: 1.0
            shadowOpacity: 0.78
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 0
        }
    }

    Canvas {
        id: glowCanvas
        anchors.fill: parent
        opacity: 1.0

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            ctx.save()
            ctx.lineCap = "round"

            // Multi-pass soft neon aura around hue ring
            for (var pass = 0; pass < 4; ++pass) {
                ctx.lineWidth = pickectx.lineWidth = picker.ringWidth + picker.usableSize * 0.055 + pass * picker.usableSize * 0.035
                ctx.globalAlpha = 0.095 - pass * 0.016

                for (var deg = 0; deg < 360; deg += 3) {
                    var a1 = deg * Math.PI / 180.0
                    var a2 = (deg + 3.5) * Math.PI / 180.0

                    ctx.beginPath()
                    ctx.arc(picker.cx, picker.cy, picker.ringMidRadius, a1, a2, false)
                    ctx.strokeStyle = picker.hsvCss(deg / 360.0, 1.0, 1.0, 1.0)
                    ctx.stroke()
                }
            }

            ctx.restore()

            // Selected hue glow
            var hueAngle = picker.hue * Math.PI * 2.0
            var hx = picker.cx + Math.cos(hueAngle) * picker.ringMidRadius
            var hy = picker.cy + Math.sin(hueAngle) * picker.ringMidRadius

            var selectedGlow = ctx.createRadialGradient(hx, hy, 2, hx, hy, picker.usableSize * 0.17)
            selectedGlow.addColorStop(0.0, picker.hsvCss(picker.hue, 1.0, 1.0, 0.62))
            selectedGlow.addColorStop(0.42, picker.hsvCss(picker.hue, 1.0, 1.0, 0.22))
            selectedGlow.addColorStop(1.0, picker.hsvCss(picker.hue, 1.0, 1.0, 0.0))

            ctx.fillStyle = selectedGlow
            ctx.beginPath()
            ctx.arc(hx, hy, picker.usableSize * 0.17, 0, Math.PI * 2)
            ctx.fill()

            // Inner disc glow
            var discGlow = ctx.createRadialGradient(
                        picker.cx,
                        picker.cy,
                        picker.discRadius * 0.12,
                        picker.cx,
                        picker.cy,
                        picker.discRadius * 1.35)

            discGlow.addColorStop(0.0, picker.hsvCss(picker.hue, 0.85, 0.95, 0.22))
            discGlow.addColorStop(0.55, picker.hsvCss(picker.hue, 1.0, 0.85, 0.10))
            discGlow.addColorStop(1.0, picker.hsvCss(picker.hue, 1.0, 0.65, 0.0))

            ctx.fillStyle = discGlow
            ctx.beginPath()
            ctx.arc(picker.cx, picker.cy, picker.discRadius * 1.35, 0, Math.PI * 2)
            ctx.fill()
        }
    }

    Canvas {
        id: wheelCanvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            ctx.save()
            ctx.lineCap = "butt"

            // Outer subtle border
            ctx.beginPath()
            ctx.arc(picker.cx, picker.cy, picker.outerRadius + 6, 0, Math.PI * 2)
            ctx.lineWidth = 2
            ctx.strokeStyle = "rgba(255,255,255,0.10)"
            ctx.stroke()

            // Main rainbow ring
            ctx.lineWidth = picker.ringWidth

            for (var deg = 0; deg < 360; deg += 1.25) {
                var a1 = deg * Math.PI / 180.0
                var a2 = (deg + 1.95) * Math.PI / 180.0

                ctx.beginPath()
                ctx.arc(picker.cx, picker.cy, picker.ringMidRadius, a1, a2, false)
                ctx.strokeStyle = picker.hsvCss(deg / 360.0, 1.0, 1.0, 1.0)
                ctx.stroke()
            }

            // Ring glossy shine
            ctx.lineWidth = 2
            for (var deg2 = 205; deg2 < 335; deg2 += 2) {
                var aa1 = deg2 * Math.PI / 180.0
                var aa2 = (deg2 + 1.8) * Math.PI / 180.0

                ctx.beginPath()
                ctx.arc(picker.cx, picker.cy, picker.ringMidRadius - picker.ringWidth * 0.34, aa1, aa2, false)
                ctx.strokeStyle = "rgba(255,255,255,0.28)"
                ctx.stroke()
            }

            // Inner ring dark glass shadow
            ctx.beginPath()
            ctx.arc(picker.cx, picker.cy, picker.ringInnerRadius - 2, 0, Math.PI * 2)
            ctx.lineWidth = 10
            ctx.strokeStyle = "rgba(0,0,0,0.34)"
            ctx.stroke()

            // Outer rim
            ctx.beginPath()
            ctx.arc(picker.cx, picker.cy, picker.outerRadius, 0, Math.PI * 2)
            ctx.lineWidth = 1.2
            ctx.strokeStyle = "rgba(255,255,255,0.24)"
            ctx.stroke()

            // Inner rim
            ctx.beginPath()
            ctx.arc(picker.cx, picker.cy, picker.ringInnerRadius, 0, Math.PI * 2)
            ctx.lineWidth = 1.1
            ctx.strokeStyle = "rgba(255,255,255,0.15)"
            ctx.stroke()

            // Tick marks around inner circle
            for (var tick = 0; tick < 360; tick += 4) {
                var major = (tick % 30) === 0
                var len = major ? 17 : 6
                var alpha = major ? 0.74 : 0.19
                var angle = tick * Math.PI / 180.0

                var r1 = picker.ringInnerRadius - 12
                var r2 = r1 - len

                ctx.beginPath()
                ctx.moveTo(picker.cx + Math.cos(angle) * r1,
                           picker.cy + Math.sin(angle) * r1)
                ctx.lineTo(picker.cx + Math.cos(angle) * r2,
                           picker.cy + Math.sin(angle) * r2)

                ctx.lineWidth = major ? 2.0 : 1.0
                ctx.strokeStyle = "rgba(255,255,255," + alpha + ")"
                ctx.stroke()
            }

            // ================================
            // Inner saturation / brightness disc
            // ================================
            ctx.save()

            ctx.beginPath()
            ctx.arc(picker.cx, picker.cy, picker.discRadius, 0, Math.PI * 2)
            ctx.clip()

            // Base hue color
            ctx.fillStyle = picker.hsvCss(picker.hue, 1.0, 1.0, 1.0)
            ctx.fillRect(
                        picker.cx - picker.discRadius,
                        picker.cy - picker.discRadius,
                        picker.discRadius * 2,
                        picker.discRadius * 2)

            // White gradient from left side
            var whiteGrad = ctx.createLinearGradient(
                        picker.cx - picker.discRadius,
                        picker.cy,
                        picker.cx + picker.discRadius,
                        picker.cy)

            whiteGrad.addColorStop(0.0, "rgba(255,255,255,1.0)")
            whiteGrad.addColorStop(0.38, "rgba(255,255,255,0.46)")
            whiteGrad.addColorStop(0.75, "rgba(255,255,255,0.05)")
            whiteGrad.addColorStop(1.0, "rgba(255,255,255,0.0)")

            ctx.fillStyle = whiteGrad
            ctx.fillRect(
                        picker.cx - picker.discRadius,
                        picker.cy - picker.discRadius,
                        picker.discRadius * 2,
                        picker.discRadius * 2)

            // Black gradient from bottom
            var blackGrad = ctx.createLinearGradient(
                        picker.cx,
                        picker.cy - picker.discRadius,
                        picker.cx,
                        picker.cy + picker.discRadius)

            blackGrad.addColorStop(0.0, "rgba(0,0,0,0.0)")
            blackGrad.addColorStop(0.42, "rgba(0,0,0,0.08)")
            blackGrad.addColorStop(0.72, "rgba(0,0,0,0.44)")
            blackGrad.addColorStop(1.0, "rgba(0,0,0,0.95)")

            ctx.fillStyle = blackGrad
            ctx.fillRect(
                        picker.cx - picker.discRadius,
                        picker.cy - picker.discRadius,
                        picker.discRadius * 2,
                        picker.discRadius * 2)

            // Glossy highlight
            var glossGrad = ctx.createRadialGradient(
                        picker.cx - picker.discRadius * 0.48,
                        picker.cy - picker.discRadius * 0.48,
                        2,
                        picker.cx - picker.discRadius * 0.18,
                        picker.cy - picker.discRadius * 0.18,
                        picker.discRadius * 1.05)

            glossGrad.addColorStop(0.0, "rgba(255,255,255,0.36)")
            glossGrad.addColorStop(0.30, "rgba(255,255,255,0.10)")
            glossGrad.addColorStop(1.0, "rgba(255,255,255,0.0)")

            ctx.fillStyle = glossGrad
            ctx.beginPath()
            ctx.arc(picker.cx, picker.cy, picker.discRadius, 0, Math.PI * 2)
            ctx.fill()

            // Slight vignette
            var vignette = ctx.createRadialGradient(
                        picker.cx,
                        picker.cy,
                        picker.discRadius * 0.22,
                        picker.cx,
                        picker.cy,
                        picker.discRadius)

            vignette.addColorStop(0.0, "rgba(0,0,0,0.0)")
            vignette.addColorStop(0.78, "rgba(0,0,0,0.04)")
            vignette.addColorStop(1.0, "rgba(0,0,0,0.30)")

            ctx.fillStyle = vignette
            ctx.beginPath()
            ctx.arc(picker.cx, picker.cy, picker.discRadius, 0, Math.PI * 2)
            ctx.fill()

            ctx.restore()

            // Disc border
            ctx.beginPath()
            ctx.arc(picker.cx, picker.cy, picker.discRadius, 0, Math.PI * 2)
            ctx.lineWidth = 1.5
            ctx.strokeStyle = "rgba(255,255,255,0.28)"
            ctx.stroke()

            // Fine inner highlight border
            ctx.beginPath()
            ctx.arc(picker.cx, picker.cy, picker.discRadius - 2, 0, Math.PI * 2)
            ctx.lineWidth = 1
            ctx.strokeStyle = "rgba(255,255,255,0.08)"
            ctx.stroke()

            ctx.restore()
        }
    }

    // Hue selector handle
    Item {
        id: hueHandle

        readonly property real angle: picker.hue * Math.PI * 2.0

        width: picker.handleSize
        height: picker.handleSize

        x: picker.cx + Math.cos(angle) * picker.ringMidRadius - width / 2
        y: picker.cy + Math.sin(angle) * picker.ringMidRadius - height / 2

        Rectangle {
            anchors.centerIn: parent
            width: picker.handleSize
            height: picker.handleSize
            radius: width / 2
            color: Qt.hsva(picker.hue, 1.0, 1.0, 1.0)
            opacity: 0.96

            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowBlur: 1.0
                shadowOpacity: 0.92
                shadowHorizontalOffset: 0
                shadowVerticalOffset: 0
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: picker.handleSize * 0.86
            height: width
            radius: width / 2
            color: Qt.rgba(1, 1, 1, 0.08)
            border.width: 3
            border.color: Qt.rgba(1, 1, 1, 0.96)
        }

        Rectangle {
            anchors.centerIn: parent
            width: picker.handleSize * 0.36
            height: width
            radius: width / 2
            color: Qt.hsva(picker.hue, 1.0, 1.0, 1.0)
        }
    }

    // Saturation / brightness selector handle
    Item {
        id: svHandle

        width: picker.svHandleSize
        height: picker.svHandleSize

        x: picker.cx - picker.discRadius
           + picker.saturation * picker.discRadius * 2.0
           - width / 2

        y: picker.cy - picker.discRadius
           + (1.0 - picker.brightness) * picker.discRadius * 2.0
           - height / 2

        Rectangle {
            anchors.centerIn: parent
            width: picker.svHandleSize
            height: picker.svHandleSize
            radius: width / 2
            color: picker.selectedColor
            opacity: 0.95

            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowBlur: 1.0
                shadowOpacity: 0.86
                shadowHorizontalOffset: 0
                shadowVerticalOffset: 0
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: picker.svHandleSize * 0.84
            height: width
            radius: width / 2
            color: Qt.rgba(0, 0, 0, 0.08)
            border.width: 3
            border.color: Qt.rgba(1, 1, 1, 0.95)
        }

        Rectangle {
            anchors.centerIn: parent
            width: picker.svHandleSize * 0.30
            height: width
            radius: width / 2
            color: picker.selectedColor
        }
    }

    // Small decorative dots inside disc
    Repeater {
        model: 5

        Rectangle {
            readonly property real px: [0.30, 0.38, 0.50, 0.61, 0.70][index]
            readonly property real py: [0.33, 0.42, 0.50, 0.58, 0.66][index]

            width: index === 2 ? 7 : 12
            height: width
            radius: width / 2

            x: picker.cx - picker.discRadius
               + px * picker.discRadius * 2.0
               - width / 2

            y: picker.cy - picker.discRadius
               + py * picker.discRadius * 2.0
               - height / 2

            color: Qt.rgba(0, 0, 0, 0.05)
            border.width: 1.3
            border.color: Qt.rgba(1, 1, 1, 0.32)
            opacity: index === 2 ? 0.50 : 0.34
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        preventStealing: true

        property bool dragging: false

        function handlePoint(px, py) {
            var dx = px - picker.cx
            var dy = py - picker.cy
            var dist = Math.sqrt(dx * dx + dy * dy)

            // Hue ring
            if (dist >= picker.ringInnerRadius && dist <= picker.outerRadius + 14) {
                var angle = Math.atan2(dy, dx)
                if (angle < 0)
                    angle += Math.PI * 2.0

                picker.hue = angle / (Math.PI * 2.0)
                return
            }

            // Inner saturation / brightness disc
            if (dist <= picker.discRadius) {
                picker.saturation = picker.clamp(
                            (px - (picker.cx - picker.discRadius))
                            / (picker.discRadius * 2.0),
                            0.0,
                            1.0)

                picker.brightness = picker.clamp(
                            1.0 - (py - (picker.cy - picker.discRadius))
                            / (picker.discRadius * 2.0),
                            0.0,
                            1.0)
            }
        }

        onPressed: function(mouse) {
            dragging = true
            picker.touchStarted()
            handlePoint(mouse.x, mouse.y)
        }

        onPositionChanged: function(mouse) {
            if (dragging)
                handlePoint(mouse.x, mouse.y)
        }

        onReleased: {
            dragging = false
            picker.touchEnded()
        }

        onCanceled: {
            dragging = false
            picker.touchEnded()
        }
    }

    Component.onCompleted: {
        wheelCanvas.requestPaint()
        glowCanvas.requestPaint()
    }
}