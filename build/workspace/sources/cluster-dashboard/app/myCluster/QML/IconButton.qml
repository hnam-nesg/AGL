import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Button {
    property string setIcon : ""
    property string setIconText : ""
    property string setIconColor : "#FFFFFF"
    property string iconBackground: "transparent"

    property real setIconSize: 25
    property real radius: height /2
    property real iconWidth: 28
    property real iconHeight: 28

    property bool isMirror: false
    property bool isGlow: false
    property bool roundIcon: false
    property bool allowUncheck: true
    property bool inputPressed: iconButtonInput.pressed
    property alias iconSource: iconSource
    property alias roundIconSource: roundIconSource

    id: control
    implicitHeight: Math.max(iconHeight, setIconSize, font.pixelSize) + 10
    implicitWidth: Math.max(iconWidth, setIconSize, font.pixelSize) + 10
    focus: false
    Accessible.focusable: false
    enabled: true
    contentItem: Label{
        text: control.text
        font: control.font
        color: control.setIconColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.NoWrap
    }

    Image{
        id:iconSource
        z:2
        visible: !roundIcon && !setIconText
        mirror: isMirror
        anchors.centerIn: parent
        source: setIcon
        scale: control.pressed || control.inputPressed ? 0.9 : 1.0
        Behavior on scale { NumberAnimation { duration: 200; } }
    }

    Image{
        id:roundIconSource
        z:2
        visible: roundIcon && !setIconText
        mirror: isMirror
        sourceSize: Qt.size(iconWidth,iconHeight)
        anchors.centerIn: parent
        source: setIcon
        scale: control.pressed || control.inputPressed ? 0.9 : 1.0
        Behavior on scale { NumberAnimation { duration: 200; } }
    }

    MouseArea {
        id: iconButtonInput
        z: 20
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        preventStealing: true

        onPressed: function(mouse) {
            console.log("IconButton pressed", control.objectName.length > 0 ? control.objectName : control.text)
            indicator.mx = mouse.x
            indicator.my = mouse.y
            anim.restart()
        }

        onClicked: function(mouse) {
            if (control.checkable && (!control.checked || control.allowUncheck))
                control.checked = !control.checked
            control.clicked()
        }
    }

    background: Rectangle {
        z:1
        anchors.fill: parent
        radius: control.radius
        color: control.iconBackground
        clip: true
        visible: true
        Behavior on color {
            ColorAnimation {
                duration: 200;
                easing.type: Easing.Linear;
            }
        }

        Rectangle {
            id: indicator
            property int mx
            property int my
            x: mx - width / 2
            y: my - height / 2
            height: width
            radius: width / 2
            color: isGlow ? Qt.lighter("#29BEB6") : Qt.lighter("#B8FF01")
        }
    }

    ParallelAnimation {
        id: anim
        NumberAnimation {
            target: indicator
            property: 'width'
            from: 0
            to: control.width * 1.5
            duration: 200
        }
        NumberAnimation {
            target: indicator;
            property: 'opacity'
            from: 0.9
            to: 0
            duration: 200
        }
    }

    onPressed: anim.restart()
}
