// GlassSliderPanel.qml
import QtQuick 2.15

Item {
    id: root
    width: 430
    height: 120
    

    property real sideInset: 0

    property real shoulderInset: 120

    property real shoulderY: 25

    property real topCurveY: 8

    property real sideBulgeX: 40

    property real sideUpperCtrlY: height * 0.2

    property real sideLowerCtrlY: height * 0.4

    property real bottomEdgeY: height - 12

    property real bottomBulge: 6

    property color topFillColor:    Qt.rgba(210/255, 245/255, 255/255, 0.26)

    property color midFillColor:    Qt.rgba(150/255, 223/255, 245/255, 0.17)

    property color lowFillColor:    Qt.rgba( 90/255, 180/255, 220/255, 0.08)

    property color bottomFadeColor: Qt.rgba( 70/255, 145/255, 190/255, 0.00)

    property color topHighlightColor: Qt.rgba(235/255, 250/255, 255/255, 0.22)

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true
        renderStrategy: Canvas.Cooperative

        function drawPanelPath(ctx) {
            var w = width
            var h = height

            var leftBottomX = root.sideInset
            var rightBottomX = w - root.sideInset

            var leftShoulderX = root.shoulderInset
            var rightShoulderX = w - root.shoulderInset

            var bottomY = root.bottomEdgeY

            ctx.beginPath()
            ctx.moveTo(leftBottomX, bottomY)

            ctx.bezierCurveTo(
                leftBottomX - root.sideBulgeX, root.sideLowerCtrlY,
                leftShoulderX - root.sideBulgeX, root.sideUpperCtrlY,
                leftShoulderX, root.shoulderY
            )

            ctx.bezierCurveTo(
                w * 0.28, root.topCurveY,
                w * 0.72, root.topCurveY,
                rightShoulderX, root.shoulderY
            )

            ctx.bezierCurveTo(
                rightShoulderX + root.sideBulgeX, root.sideUpperCtrlY,
                rightBottomX + root.sideBulgeX, root.sideLowerCtrlY,
                rightBottomX, bottomY
            )

            ctx.bezierCurveTo(
                w * 0.76, bottomY + root.bottomBulge,
                w * 0.24, bottomY + root.bottomBulge,
                leftBottomX, bottomY
            )

            ctx.closePath()
        }

        onPaint: {
            var ctx = getContext("2d")
            var w = width
            var h = height

            ctx.clearRect(0, 0, w, h)

            drawPanelPath(ctx)

            var fillGrad = ctx.createLinearGradient(0, 0, 0, h)
            fillGrad.addColorStop(0.00, root.topFillColor)
            fillGrad.addColorStop(0.22, root.midFillColor)
            fillGrad.addColorStop(0.58, root.lowFillColor)
            fillGrad.addColorStop(1.00, root.bottomFadeColor)
            ctx.fillStyle = fillGrad
            ctx.fill()

            ctx.save()
            drawPanelPath(ctx)
            ctx.clip()

            var leftGlow = ctx.createRadialGradient(
                w * 0.18, h * 0.22, 4,
                w * 0.18, h * 0.22, w * 0.25
            )
            leftGlow.addColorStop(0.00, "rgba(120,255,255,0.22)")
            leftGlow.addColorStop(0.35, "rgba(100,220,255,0.10)")
            leftGlow.addColorStop(1.00, "rgba(100,220,255,0.00)")
            ctx.fillStyle = leftGlow
            ctx.fillRect(0, 0, w, h)

            var topLight = ctx.createLinearGradient(0, 0, 0, h * 0.42)
            topLight.addColorStop(0.00, "rgba(255,255,255,0.15)")
            topLight.addColorStop(0.35, "rgba(255,255,255,0.05)")
            topLight.addColorStop(1.00, "rgba(255,255,255,0.00)")
            ctx.fillStyle = topLight
            ctx.fillRect(0, 0, w, h * 0.42)

            var mist = ctx.createRadialGradient(
                w * 0.50, h * 0.80, 8,
                w * 0.50, h * 0.80, w * 0.30
            )
            mist.addColorStop(0.00, "rgba(255,255,255,0.08)")
            mist.addColorStop(0.45, "rgba(255,255,255,0.03)")
            mist.addColorStop(1.00, "rgba(255,255,255,0.00)")
            ctx.fillStyle = mist
            ctx.fillRect(0, 0, w, h)

            ctx.restore()

            drawPanelPath(ctx)
            var strokeGrad = ctx.createLinearGradient(0, 0, 0, h)
            strokeGrad.addColorStop(0.00, "rgba(225,250,255,0.28)")
            strokeGrad.addColorStop(0.25, "rgba(170,230,250,0.16)")
            strokeGrad.addColorStop(0.65, "rgba(110,190,230,0.07)")
            strokeGrad.addColorStop(1.00, "rgba(80,160,210,0.00)")
            ctx.strokeStyle = strokeGrad
            ctx.lineWidth = 1.2
            ctx.stroke()
        }

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        Component.onCompleted: requestPaint()
    }
}
