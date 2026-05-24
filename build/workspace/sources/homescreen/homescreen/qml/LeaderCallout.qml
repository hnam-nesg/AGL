// LeaderCallout.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Shapes
import QtQuick.Effects

Item {
    id: root
    property int x_: 0
    property int y_: 0
    property point anchor: Qt.point(100,100)  // điểm bám trên vật thể
    property point elbow:  Qt.point(anchor.x+40, anchor.y-30) // gấp khúc
    property point labelPos: Qt.point(elbow.x+80, elbow.y-40) // nơi đặt chữ
    property string text: "label"
    property color lineColor: "#C8C8C8"
    property real  lineWidth: 2
    property bool  dashed: false
    property real  haloRadius: 15   // vòng tròn lớn (trong suốt)
    property real  dotRadius: 15/2     // chấm trắng ở tâm
    opacity: 0
    property alias checked_: button_display.checked

    width: Math.max(anchor.x, elbow.x, labelPos.x) + 40
    height: Math.max(anchor.y, elbow.y, labelPos.y) + 40
    // Nét chỉ dẫn: anchor -> elbow -> labelPos
    Shape {
        anchors.fill: parent
        ShapePath {
            strokeWidth: root.lineWidth
            strokeColor: root.lineColor
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            strokeStyle: dashed ? ShapePath.DashLine : ShapePath.SolidLine
            //dashPattern: [6, 5]    // nét đứt
            startX: root.anchor.x; startY: root.anchor.y
            PathLine { x: root.elbow.x;  y: root.elbow.y }
            PathLine { x: root.labelPos.x; y: root.labelPos.y }
        }
    }

    // Vòng tròn đánh dấu điểm anchor (2 vòng đồng tâm)
    Canvas {
        anchors.fill: parent
        onPaint: {
            const ctx = getContext("2d"); ctx.reset();
            // halo mờ
            ctx.beginPath();
            ctx.arc(root.anchor.x, root.anchor.y, root.haloRadius, 0, Math.PI*2);
            ctx.strokeStyle = root.lineColor;
            ctx.lineWidth = 1.2;
            ctx.globalAlpha = 0.45;
            ctx.stroke();
            // chấm trắng
            ctx.beginPath();
            ctx.arc(root.anchor.x, root.anchor.y, root.dotRadius, 0, Math.PI*2);
            ctx.fillStyle = "white";
            ctx.globalAlpha = 1;
            ctx.fill();
        }
    }

    // Nhãn
    ButtonControl {
        id: button
        x: root.labelPos.x + root.x_
        y: root.labelPos.y + root.y_
        contentText: root.text
        activeText: true
        fontSize: 20
        enabled: false; align: true
    }
    Button {
        id: button_display
        height: 40; width: 40; x: root.anchor.x - 20; y: root.anchor.y - 20
        background: Rectangle{
            anchors.centerIn: parent; width: 15; height: 15
            radius: 15/2; color: "#8aa1a1a1"
            Rectangle {
                anchors.fill: parent
                scale: 1.1
                color: "white"
                radius: 15/2
                layer.enabled: true
                layer.effect: MultiEffect {
                    blurEnabled: true
                    blur:0.6
                }
                BorderColor{borderWidth: 3; c1: "#00E5FF"; c2: "#00E5FF";/*c1: "white"; c2: "white";*/ radius: 15/2}
            }
        }
        enabled: true; checkable: true
    }

    SequentialAnimation on opacity{
        // PauseAnimation {
        //     duration: 2000
        // }
        OpacityAnimator{from: 0; to: 1; loops: 1; duration: 2000; easing.type: Easing.InOutQuad}

    }
}
