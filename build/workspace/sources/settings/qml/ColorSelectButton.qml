import QtQuick
import QtQuick.Effects
import QtQuick.Controls

Button {
    id: btn

    property color buttonColor: "#00FFFF"
    property bool active: false

    implicitHeight: 46
    font.family: fontLoader.name
    font.pixelSize: 16
    font.bold: true

    contentItem: Text {
        text: btn.text
        color: "white"
        font: btn.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowBlur: 0.55
            shadowOpacity: 0.85
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 1
        }
    }

    background: Rectangle {
        radius: 15
        color: btn.buttonColor
        border.width: btn.active ? 3 : 1
        border.color: btn.active ? Qt.rgba(1, 1, 1, 0.95)
                                 : Qt.rgba(1, 1, 1, 0.26)

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowBlur: btn.active ? 1.0 : 0.65
            shadowOpacity: btn.active ? 0.95 : 0.55
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 0
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.18)
        }

        Rectangle {
            x: 4
            y: 4
            width: parent.width - 8
            height: parent.height * 0.42
            radius: parent.radius - 4
            color: Qt.rgba(1, 1, 1, btn.active ? 0.24 : 0.14)
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: btn.down ? Qt.rgba(0, 0, 0, 0.22)
                            : btn.hovered ? Qt.rgba(1, 1, 1, 0.08)
                                          : Qt.rgba(0, 0, 0, 0.0)
        }
    }
}