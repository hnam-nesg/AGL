import QtQuick
import QtQuick.Effects
import Qt5Compat.GraphicalEffects

Item {
    id: root
    width: 420
    height: 70

    FontLoader{
        id: button_home
        source: "images/Roboto-BoldCondensed.ttf"
    }

    signal disableSwipeView()
    signal enableSwipeView()
    // -3..+3
    property int fromValue: -3
    property int toValue: 3
    property int stepValue: 1
    property int value: 0

    // style
    property color leftColor:  "#008DFF"
    property color rightColor: "#FF0000"
    property color textColor:  "white"

    property int sidePadding: 16
    property int gap: 8
    property real thin: 2
    property real thick: 10
    property real dotSize: 20

    readonly property int stepsCount: (toValue - fromValue) / stepValue + 1
    readonly property real barW: width - sidePadding * 2
    readonly property real centerX: sidePadding + barW / 2

    function valueToX(v) {
        var idx = (v - fromValue) / stepValue;
        return sidePadding + (idx / (stepsCount - 1)) * barW;
    }
    function xToNearestValue(x) {
        var t = (x - sidePadding) / barW;
        t = Math.max(0, Math.min(1, t));
        var idx = Math.round(t * (stepsCount - 1));
        return fromValue + idx * stepValue;
    }

    function selectedColor() {
        if (value < 0) return leftColor;
        if (value > 0) return rightColor;
        return "white";
    }

    // ===== Labels =====
    Row {
        id: labels
        x: root.sidePadding
        width: root.barW
        y: 0
        height: 22

        Repeater {
            model: root.stepsCount
            delegate: Item {
                width: labels.width / (root.stepsCount - 1)
                height: labels.height

                readonly property int v: root.fromValue + index * root.stepValue
                readonly property bool isSel: v === root.value

                Text {
                    anchors.horizontalCenter: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    text: Math.abs(v).toString()
                    font.family: button_home.name
                    // số được chọn: to + đổi màu theo thanh
                    font.bold: true
                    font.pixelSize: isSel ? 35 : 20
                    color: isSel
                           ? (v < 0 ? root.leftColor : (v > 0 ? root.rightColor : "white"))
                           : root.textColor
                    opacity: isSel ? 1.0 : 0.9

                    Behavior on font.pixelSize { NumberAnimation { duration: 120 } }
                    Behavior on color { ColorAnimation { duration: 120 } }
                }
            }
        }
    }

    // ===== Bar taper + gap + round caps =====
    Canvas {
        id: canvas
        anchors.left: parent.left
        anchors.right: parent.right
        y: 35
        height: 25

        function thicknessAt(x) {
            var dist = Math.abs(x - root.centerX);
            var maxDist = root.barW / 2;
            var t = dist / maxDist; // 0..1
            return root.thin + (root.thick - root.thin) * t;
        }

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            var yMid = height / 2;
            var gapHalf = root.gap / 2;

            var xL0 = root.sidePadding;
            var xL1 = root.centerX - gapHalf;

            var xR0 = root.centerX + gapHalf;
            var xR1 = root.sidePadding + root.barW;

            // vẽ thân (không cap), sau đó vẽ cap tròn riêng => tròn chuẩn
            function drawBody(x0, x1, color) {
                var th0 = canvas.thicknessAt(x0);
                var th1 = canvas.thicknessAt(x1);
                ctx.beginPath();
                ctx.moveTo(x0, yMid - th0/2);
                ctx.lineTo(x1, yMid - th1/2);
                ctx.lineTo(x1, yMid + th1/2);
                ctx.lineTo(x0, yMid + th0/2);
                ctx.closePath();
                ctx.fillStyle = color;
                ctx.fill();
                return { th0: th0, th1: th1 };
            }

            function drawCap(x, th, color) {
                ctx.beginPath();
                ctx.arc(x, yMid, th/2, 0, Math.PI * 2);
                ctx.closePath();
                ctx.fillStyle = color;
                ctx.fill();
            }

            // Left
            var l = drawBody(xL0, xL1, root.leftColor);
            drawCap(xL0, l.th0, root.leftColor);   // cap trái tròn
            // (không vẽ cap ở xL1 để giữ khe hở giữa)

            // Right
            var r = drawBody(xR0, xR1, root.rightColor);
            drawCap(xR1, r.th1, root.rightColor);  // cap phải tròn
        }

        Component.onCompleted: requestPaint()
        onWidthChanged: requestPaint()
        Connections {
            target: root
            function onThinChanged() { canvas.requestPaint() }
            function onThickChanged() { canvas.requestPaint() }
            function onGapChanged() { canvas.requestPaint() }
        }
    }

    // ===== White dot (snap) =====
    Rectangle {
        id: dot
        width: root.dotSize
        height: root.dotSize
        radius: width/2
        color: "white"
        y: canvas.y + canvas.height/2 - height/2
        x: root.valueToX(root.value) - width/2

        Behavior on x { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
    }

    // ===== Click/drag snap =====
    MouseArea {
        anchors.left: parent.left
        anchors.right: parent.right
        y: canvas.y - 6
        height: canvas.height + 100
        hoverEnabled: true

        onPressed: {root.value = root.xToNearestValue(mouse.x)}
        onPositionChanged: {if (pressed) root.value = root.xToNearestValue(mouse.x)}
        onEntered: {root.enableSwipeView()}
        onExited: {root.disableSwipeView()}
    }
}
