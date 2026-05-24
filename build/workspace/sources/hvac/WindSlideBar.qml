// NumberStepSlider.qml
import QtQuick 2.15

Item {
    id: root
    width: 420
    height: 90
    
    signal disableSwipeView()
    signal enableSwipeView()

    FontLoader{
        id: button_home
        source: "images/Roboto-BoldCondensed.ttf"
    }

    property int from: 0
    property int to: 6
    property int value: 0

    property color activeColor: "#6ED6FF"
    property color normalColor: "#EDEDED"
    property color trackColor: "#33C7FF"
    property color glowColor: "#143A52"

    readonly property int stepCount: to - from + 1
    readonly property real leftPadding: 10
    readonly property real rightPadding: 10
    readonly property real trackWidth: width - leftPadding - rightPadding
    readonly property real stepSpacing: stepCount > 1 ? trackWidth / (stepCount - 1) : 0

    // Tâm theo trục Y của thanh
    property real trackCenterY: 58

    // Độ dày thanh chính: trái mảnh, phải dày hơn
    property real trackLeftThickness: 1
    property real trackRightThickness: 6

    // Độ dày vùng glow phía sau
    property real glowLeftThickness: 5
    property real glowRightThickness: 12

    function xForValue(v) {
        return leftPadding + (v - from) * stepSpacing
    }

    function nearestValue(mouseX) {
        var raw = Math.round((mouseX - leftPadding) / stepSpacing) + from
        return Math.max(from, Math.min(to, raw))
    }

    Canvas {
        id: glowTrack
        anchors.fill: parent
        antialiasing: true

        function drawTaperedBar(ctx, x, centerY, width, leftThickness, rightThickness, fillStyle) {
            var l = leftThickness
            var r = rightThickness

            var leftTopY = centerY - l / 2
            var rightTopY = centerY - r / 2
            var leftBottomY = centerY + l / 2
            var rightBottomY = centerY + r / 2

            var leftRadius = l / 2
            var rightRadius = r / 2

            ctx.beginPath()

            // bắt đầu từ đầu trái phía trên
            ctx.moveTo(x + leftRadius, leftTopY)

            // cạnh trên đi sang phải, dày dần
            ctx.bezierCurveTo(
                x + width * 0.30, leftTopY,
                x + width * 0.72, rightTopY,
                x + width - rightRadius, rightTopY
            )

            // bo tròn đầu phải
            ctx.quadraticCurveTo(
                x + width, centerY,
                x + width - rightRadius, rightBottomY
            )

            // cạnh dưới quay về trái
            ctx.bezierCurveTo(
                x + width * 0.72, rightBottomY,
                x + width * 0.30, leftBottomY,
                x + leftRadius, leftBottomY
            )

            // bo tròn đầu trái
            ctx.quadraticCurveTo(
                x, centerY,
                x + leftRadius, leftTopY
            )

            ctx.closePath()
            ctx.fillStyle = fillStyle
            ctx.fill()
        }

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()

            drawTaperedBar(
                ctx,
                root.leftPadding,
                root.trackCenterY,
                root.trackWidth,
                root.glowLeftThickness,
                root.glowRightThickness,
                root.glowColor
            )
        }

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        Component.onCompleted: requestPaint()
    }

    Canvas {
        id: track
        anchors.fill: parent
        antialiasing: true

        function drawTaperedBar(ctx, x, centerY, width, leftThickness, rightThickness, fillStyle) {
            var l = leftThickness
            var r = rightThickness

            var leftTopY = centerY - l / 2
            var rightTopY = centerY - r / 2
            var leftBottomY = centerY + l / 2
            var rightBottomY = centerY + r / 2

            var leftRadius = l / 2
            var rightRadius = r / 2

            ctx.beginPath()

            // bắt đầu từ đầu trái phía trên
            ctx.moveTo(x + leftRadius, leftTopY)

            // cạnh trên đi sang phải, dày dần
            ctx.bezierCurveTo(
                x + width * 0.30, leftTopY,
                x + width * 0.72, rightTopY,
                x + width - rightRadius, rightTopY
            )

            // bo đầu phải
            ctx.quadraticCurveTo(
                x + width, centerY,
                x + width - rightRadius, rightBottomY
            )

            // cạnh dưới quay về trái
            ctx.bezierCurveTo(
                x + width * 0.72, rightBottomY,
                x + width * 0.30, leftBottomY,
                x + leftRadius, leftBottomY
            )

            // bo đầu trái
            ctx.quadraticCurveTo(
                x, centerY,
                x + leftRadius, leftTopY
            )

            ctx.closePath()
            ctx.fillStyle = fillStyle
            ctx.fill()
        }

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()

            drawTaperedBar(
                ctx,
                root.leftPadding,
                root.trackCenterY,
                root.trackWidth,
                root.trackLeftThickness,
                root.trackRightThickness,
                root.trackColor
            )
        }

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        Component.onCompleted: requestPaint()
    }

    Repeater {
        model: root.stepCount

        delegate: Text {
            property int stepValue: root.from + index
            property bool selected: stepValue === root.value

            text: stepValue
            anchors.verticalCenter: labelGuide.verticalCenter
            x: root.xForValue(stepValue) - width / 2
            color: selected ? root.activeColor : root.normalColor
            font.pixelSize: selected ? 40 : 20
            font.family: button_home.name
            opacity: selected ? 1.0 : 0.9

            Behavior on font.pixelSize {
                NumberAnimation { duration: 140 }
            }
            Behavior on color {
                ColorAnimation { duration: 140 }
            }
        }
    }

    Item {
        id: labelGuide
        x: 0
        y: 10
        width: parent.width
        height: 24
    }

    Rectangle {
        id: knobGlow
        width: 26
        height: 26
        radius: width / 2
        y: root.trackCenterY - height / 2 - 2
        x: root.xForValue(root.value) - width / 2
        color: "#66FFFFFF"
        opacity: 0.18

        Behavior on x {
            NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
        }
    }

    Rectangle {
        id: knob
        width: 18
        height: 18
        radius: width / 2
        y: root.trackCenterY - height / 2
        x: root.xForValue(root.value) - width / 2
        color: "white"
        border.color: "#D9D9D9"
        border.width: 1

        Behavior on x {
            NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onPressed: root.value = root.nearestValue(mouse.x)
        onPositionChanged: {
            if (pressed)
                root.value = root.nearestValue(mouse.x)
        }

        onEntered: {root.enableSwipeView()}
        onExited: {root.disableSwipeView()}
    }
}
