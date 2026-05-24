import QtQuick
import QtQuick.Controls.Basic as Control
import Qt5Compat.GraphicalEffects

Control.Button {
    id: button
    width: 46
    height: 46
    focusPolicy: Qt.NoFocus

    property string iconName: "search"
    property color iconColor: "#3c4043"
    property color buttonColor: "#ffffff"
    property color pressedColor: "#e8f0fe"

    background: Rectangle {
        radius: Math.min(width, height) / 2
        color: button.pressed ? button.pressedColor
              : button.hovered ? "#f8fafd"
              : button.buttonColor

        border.width: 1
        border.color: "#dadce0"

        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 0
            verticalOffset: 2
            radius: 10
            samples: 24
            color: "#33000000"
        }
    }

    contentItem: Item {
        MapIcon {
            width: Math.min(24, parent.width - 10)
            height: width
            anchors.centerIn: parent
            iconName: button.iconName
            iconColor: button.iconColor
        }
    }
}