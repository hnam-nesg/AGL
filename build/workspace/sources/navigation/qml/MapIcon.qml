import QtQuick

Canvas {
    id: icon
    width: 24
    height: 24
    antialiasing: true

    property string iconName: "search"
    property color iconColor: "#3c4043"

    onIconNameChanged: requestPaint()
    onIconColorChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()

    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        ctx.strokeStyle = iconColor
        ctx.fillStyle = iconColor
        ctx.lineWidth = 2.3
        ctx.lineCap = "round"
        ctx.lineJoin = "round"

        var cx = width / 2
        var cy = height / 2

        if (iconName === "search") {
            ctx.beginPath()
            ctx.arc(cx - 2, cy - 2, 6.2, 0, Math.PI * 2)
            ctx.stroke()
            ctx.beginPath()
            ctx.moveTo(cx + 3, cy + 3)
            ctx.lineTo(width - 4, height - 4)
            ctx.stroke()
            return
        }

        if (iconName === "plus" || iconName === "minus") {
            ctx.beginPath()
            ctx.moveTo(5, cy)
            ctx.lineTo(width - 5, cy)
            ctx.stroke()

            if (iconName === "plus") {
                ctx.beginPath()
                ctx.moveTo(cx, 5)
                ctx.lineTo(cx, height - 5)
                ctx.stroke()
            }
            return
        }

        if (iconName === "3d") {
            ctx.lineWidth = 2.1

            ctx.beginPath()
            ctx.moveTo(cx, 3)
            ctx.lineTo(width - 5, 8)
            ctx.lineTo(width - 5, height - 7)
            ctx.lineTo(cx, height - 2.5)
            ctx.lineTo(5, height - 7)
            ctx.lineTo(5, 8)
            ctx.closePath()
            ctx.stroke()

            ctx.beginPath()
            ctx.moveTo(cx, 3)
            ctx.lineTo(cx, 11)
            ctx.lineTo(width - 5, 8)
            ctx.moveTo(cx, 11)
            ctx.lineTo(5, 8)
            ctx.moveTo(cx, 11)
            ctx.lineTo(cx, height - 2.5)
            ctx.stroke()
            return
        }

        if (iconName === "2d") {
            ctx.lineWidth = 2.1

            ctx.beginPath()
            ctx.moveTo(4, 7)
            ctx.lineTo(cx, 3.5)
            ctx.lineTo(width - 4, 7)
            ctx.lineTo(cx, 10.5)
            ctx.closePath()
            ctx.stroke()

            ctx.beginPath()
            ctx.moveTo(4, 12)
            ctx.lineTo(cx, 8.5)
            ctx.lineTo(width - 4, 12)
            ctx.moveTo(4, 17)
            ctx.lineTo(cx, 13.5)
            ctx.lineTo(width - 4, 17)
            ctx.stroke()
            return
        }

        if (iconName === "location") {
            ctx.beginPath()
            ctx.arc(cx, cy, 6.5, 0, Math.PI * 2)
            ctx.stroke()

            ctx.beginPath()
            ctx.arc(cx, cy, 2.2, 0, Math.PI * 2)
            ctx.fill()

            ctx.beginPath()
            ctx.moveTo(cx, 2.5)
            ctx.lineTo(cx, 6)
            ctx.moveTo(cx, height - 2.5)
            ctx.lineTo(cx, height - 6)
            ctx.moveTo(2.5, cy)
            ctx.lineTo(6, cy)
            ctx.moveTo(width - 2.5, cy)
            ctx.lineTo(width - 6, cy)
            ctx.stroke()
            return
        }
    }
}
