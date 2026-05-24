// NeonComboBox.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Effects

ComboBox {
    id: combo

    property color panelColor: "#101826"
    property color borderColor: "#42546B"
    property color glowColor: "#7FB0FF"
    property color textColor: "#FFFFFF"
    property color disabledTextColor: "#6B7280"

    // Truyền font từ ngoài vào, ví dụ: uiFontFamily: fontLoader.name
    property string uiFontFamily: ""

    implicitHeight: 48

    font.family: uiFontFamily
    font.pixelSize: 18

    palette.text: "white"
    palette.buttonText: "white"
    palette.windowText: "white"
    palette.highlightedText: "white"
    palette.brightText: "white"

    contentItem: Text {
        leftPadding: 18
        rightPadding: 42

        text: combo.displayText
        color: combo.enabled ? combo.textColor : combo.disabledTextColor

        font.family: combo.uiFontFamily
        font.pixelSize: combo.font.pixelSize

        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        elide: Text.ElideRight
    }

    indicator: Canvas {
        id: comboArrow

        x: combo.width - width - 16
        y: Math.round((combo.height - height) / 2)

        width: 16
        height: 10

        rotation: combo.popup.visible ? 180 : 0

        Behavior on rotation {
            NumberAnimation {
                duration: 140
                easing.type: Easing.OutCubic
            }
        }

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            ctx.beginPath()
            ctx.moveTo(1, 1)
            ctx.lineTo(width / 2, height - 1)
            ctx.lineTo(width - 1, 1)

            ctx.lineWidth = 2.2
            ctx.lineCap = "round"
            ctx.lineJoin = "round"
            ctx.strokeStyle = combo.enabled ? "#FFFFFF" : "#6B7280"
            ctx.stroke()
        }

        Connections {
            target: combo

            function onEnabledChanged() {
                comboArrow.requestPaint()
            }

            function onPressedChanged() {
                comboArrow.requestPaint()
            }

            function onDownChanged() {
                comboArrow.requestPaint()
            }

            function onVisibleChanged() {
                comboArrow.requestPaint()
            }
        }
    }

    background: Rectangle {
        radius: 14

        color: combo.enabled ? combo.panelColor : "#111827"

        border.width: combo.popup.visible || combo.activeFocus ? 2 : 1
        border.color: combo.popup.visible || combo.activeFocus
                      ? combo.glowColor
                      : combo.borderColor

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowBlur: combo.popup.visible || combo.activeFocus ? 0.95 : 0.45
            shadowOpacity: combo.popup.visible || combo.activeFocus ? 0.75 : 0.35
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 0
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 4

            height: parent.height * 0.42
            radius: parent.radius - 4

            color: Qt.rgba(1, 1, 1, combo.enabled ? 0.08 : 0.03)
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius

            color: combo.down ? Qt.rgba(0, 0, 0, 0.16)
                              : Qt.rgba(0, 0, 0, 0.0)
        }
    }

    delegate: Item {
        id: delegateRoot

        width: combo.width
        height: 44

        readonly property bool isCurrent: combo.currentIndex === index

        Rectangle {
            anchors.fill: parent
            anchors.margins: 2

            radius: 10

            color: delegateRoot.isCurrent
                   ? Qt.rgba(0.50, 0.69, 1.0, 0.32)
                   : itemMouse.containsMouse
                     ? Qt.rgba(0.50, 0.69, 1.0, 0.18)
                     : Qt.rgba(0.06, 0.10, 0.16, 0.96)

            border.width: delegateRoot.isCurrent || itemMouse.containsMouse ? 1 : 0
            border.color: Qt.rgba(0.70, 0.84, 1.0, 0.40)
        }

        Text {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 12

            text: modelData
            color: "#FFFFFF"

            font.family: combo.uiFontFamily
            font.pixelSize: 17

            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight
        }

        MouseArea {
            id: itemMouse

            anchors.fill: parent
            hoverEnabled: true

            onClicked: {
                combo.currentIndex = index
                combo.activated(index)
                combo.popup.close()
            }
        }
    }

    popup: Popup {
        id: comboPopup

        y: combo.height + 6
        width: combo.width

        padding: 6
        implicitHeight: Math.min(contentItem.implicitHeight + 12, 240)

        background: Rectangle {
            radius: 14
            color: "#0B1220"

            border.width: 1
            border.color: "#3B536E"

            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowBlur: 1.0
                shadowOpacity: 0.80
                shadowHorizontalOffset: 0
                shadowVerticalOffset: 4
            }
        }

        contentItem: ListView {
            id: popupList

            clip: true
            implicitHeight: contentHeight

            model: combo.popup.visible ? combo.delegateModel : null
            currentIndex: combo.highlightedIndex

            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }
        }
    }
}