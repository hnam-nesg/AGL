import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Qt5Compat.GraphicalEffects
Button {
    id: root

    property bool press: root.pressed
    property string source: ""
    property string source_: ""
    property string source_v: ""
    property string source__: ""
    property int fontSize: 0
    property int rotation: 0
    property bool activeText: false
    property string contentText: ""
    property string text_color: "white"
    property string text_sub: ""
    property int text_direct: 0
    property bool align: false
    property int width_image: 0
    property int height_image: 0
    property int x_text: 0

    property bool incomingCall: false
    property string btn_call: ""

    FontLoader{
        id: button_home
        source: "images/Roboto-BoldCondensed.ttf"
    }
    enabled: true
    checkable: true
    background: Rectangle{
        id: rect_
        visible: true
        anchors.fill: parent
        color: "transparent"
        // scale: 1
        // border.color: Qt.darker(color, 1.1)
        // layer.enabled: true
        // layer.effect: DropShadow {
        //     horizontalOffset: 0; verticalOffset: 0; radius: 25
        //     samples: 20; color: "#80FFFFFF"
        // }
    }

    Image{
        id: image
        //anchors.fill: parent
        z: 3; width: root.width_image; height: root.height_image
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        source: root.source
        smooth: true
        fillMode: Image.PreserveAspectCrop
        NumberAnimation on scale{duration: 200; easing.type: Easing.Linear}
        scale: root.pressed ? 0.8 : 1
        transformOrigin: Item.Center

        SequentialAnimation on rotation {
            running: root.incomingCall
            loops: Animation.Infinite

            NumberAnimation { to: -18; duration: 80 }
            NumberAnimation { to: 18; duration: 80 }
            NumberAnimation { to: -14; duration: 70 }
            NumberAnimation { to: 14; duration: 70 }
            NumberAnimation { to: -8; duration: 60 }
            NumberAnimation { to: 8; duration: 60 }
            NumberAnimation { to: 0; duration: 80 }
            PauseAnimation { duration: 400 }
        }
    }

    Text{
        width: 10; height: 10
        text: root.text_sub
        anchors.horizontalCenter: image.horizontalCenter
        anchors.verticalCenter: image.verticalCenter
        font.pixelSize: 20
        font.family: button_home.name
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.verticalCenterOffset: root.text_direct
    }

    Text{
        id: text
        x: root.x_text; width: parent.width; height: parent.height
        textFormat: Text.RichText
        color: root.text_color
        text: root.contentText
        font.pixelSize: 20
        font.family: button_home.name
        horizontalAlignment: root.align ? Text.AlignLeft : Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Image{
        id: image_v
        x: 10; width: 30; height: 30
        anchors.verticalCenter: text.verticalCenter
        source: root.source_v
        rotation: root.rotation
    }

    Rectangle {
        anchors.fill: parent
        radius: parent.width / 2
        color: root.btn_call
        opacity: 0.25
        anchors.centerIn: callButton
        visible: incomingCall

        SequentialAnimation on scale {
            running: incomingCall
            loops: Animation.Infinite
            NumberAnimation { from: 1.0; to: 2.0; duration: 1200 }
        }

        SequentialAnimation on opacity {
            running: incomingCall
            loops: Animation.Infinite
            NumberAnimation { from: 0.25; to: 0.0; duration: 1200 }
        }
    }
}