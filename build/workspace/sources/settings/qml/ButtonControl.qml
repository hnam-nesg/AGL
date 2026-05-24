import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Qt5Compat.GraphicalEffects

Button {
    id: root

    property bool press: root.pressed
    property string source: ""
    property string source_v: ""
    property int fontSize: 0
    property bool activeText: false
    property string contentText: ""
    property bool align: false
    property int width_image: 0
    property int height_image: 0
    property int x_text: 0

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
    }

    Image{
        id: image
        z: 3; width: root.width_image; height: root.height_image
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        source: root.source
        smooth: true
        fillMode: Image.PreserveAspectCrop
        NumberAnimation on scale{duration: 200; easing.type: Easing.Linear}
        scale: root.pressed ? 0.8 : 1
    }

    Text{
        id: text
        x: root.x_text; width: parent.width; height: parent.height
        textFormat: Text.RichText
        color: "white"
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
}

