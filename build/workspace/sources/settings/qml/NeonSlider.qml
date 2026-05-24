import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects

Slider {
    id: slider

    property color trackColor: "#1A2636"
    property color fillColor: root.accentColor
    property color handleColor: "#EAF2FF"
    property color handleInnerColor: root.accentColor

    implicitHeight: 42

    background: Item {
        x: slider.leftPadding
        y: Math.round((slider.height - height) / 2)
        width: slider.availableWidth
        height: 16

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width
            height: 8
            radius: 4
            color: slider.trackColor
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.08)
        }

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: slider.visualPosition * parent.width
            height: 8
            radius: 4
            color: slider.fillColor

            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowBlur: 0.85
                shadowOpacity: 0.75
                shadowHorizontalOffset: 0
                shadowVerticalOffset: 0
            }
        }

        Rectangle {
            x: slider.visualPosition * parent.width - 2
            anchors.verticalCenter: parent.verticalCenter
            width: 4
            height: 18
            radius: 2
            color: Qt.rgba(1, 1, 1, 0.55)
        }
    }

    handle: Item {
        x: slider.leftPadding + slider.visualPosition * slider.availableWidth - width / 2
        y: Math.round((slider.height - height) / 2)
        width: slider.pressed ? 30 : 26
        height: width

        Behavior on width {
            NumberAnimation { duration: 110; easing.type: Easing.OutCubic }
        }

        Rectangle {
            anchors.fill: parent
            radius: width / 2
            color: slider.handleColor
            border.width: 2
            border.color: Qt.rgba(1, 1, 1, 0.95)

            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowBlur: slider.pressed ? 1.0 : 0.70
                shadowOpacity: slider.pressed ? 0.95 : 0.70
                shadowHorizontalOffset: 0
                shadowVerticalOffset: 0
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.42
            height: width
            radius: width / 2
            color: slider.handleInnerColor
        }
    }
}