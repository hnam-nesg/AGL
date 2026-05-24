import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Qt5Compat.GraphicalEffects
Item{
    id: root
    // anchors.fill: parent

    property string contentText: ""
    property int fontSize: 16
    property string mode: ""
    property alias check: control.checked
    property alias x_: control.x
    property alias y_: control.y

    FontLoader{
        id: font_text
        source: "images/Roboto-BoldCondensed.ttf"
    }

    Text{
        id: text
        x: 10
        y: 10
        width: 100
        height: 40
        color: "white"
        text: root.contentText
        font.pixelSize: 20
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        font.family: font_text.name
    }

    MultiEffect {
        id: effect_text
        anchors.fill: text
        source: text
        shadowEnabled: true
        shadowColor: "black"
        shadowOpacity: 1
        shadowBlur: 0.5
        shadowHorizontalOffset: 5
        shadowVerticalOffset: 5
    }

    Switch {
        id: control
        width: 80
        height: 40
        x: 120
        y: 10

        //Behavior on x {PropertyAnimation {duration: 200; easing.type: Easing.InOutQuad}}

        indicator: Rectangle {
            id: track
            anchors.centerIn: parent
            width: control.width
            height: control.height
            radius: 8
            color: root.check ? "#202a36" : "#444"
            border.color: Qt.darker(color, 1.2)
            layer.enabled: root.check
            layer.effect: DropShadow {
                horizontalOffset: 0; verticalOffset: 0
                radius: 8; samples: 15; color: "#80FFFFFF"
            }

            Rectangle {
                id: knob
                width: 18
                height: 36
                radius: 8
                color: root.check ? "white" : "grey"
                anchors.verticalCenter: parent.verticalCenter
                x: root.check ? parent.width - width : 0
                Behavior on x {
                    NumberAnimation { duration: 180; easing.type: Easing.InOutQuad }
                }

                layer.enabled: root.check
                layer.effect: DropShadow {
                    horizontalOffset: 0; verticalOffset: 0
                    radius: 8; samples: 15; color: "#80FFFFFF"
                }
            }

            Behavior on color { ColorAnimation { duration: 180; easing.type: Easing.InOutQuad } }
        }

        contentItem: Text {
            text: root.mode
            color: "white"
            font.pixelSize: root.fontSize
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.family: font_text.name        }
    }
}
