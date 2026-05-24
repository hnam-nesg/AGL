import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Qt5Compat.GraphicalEffects
Button {
    id: root
    Behavior on height_image{NumberAnimation{duration: 300; easing.type: Easing.Linear}}
    Behavior on height{NumberAnimation{duration: 300; easing.type: Easing.Linear}}
    property string source: ""
    property string source_v: ""
    property bool activeText: false
    property string contentText: ""
    property string text_color: "white"
    property int width_image: 0
    property int height_image: 0

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
        width: root.width_image; height: root.height_image
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        source: root.source
        smooth: true
        fillMode: Image.PreserveAspectCrop
        NumberAnimation on scale{duration: 200; easing.type: Easing.Linear}
        scale: root.pressed ? 0.8 : 1
    }

    Image{
        id: image_v
        width: 24; height: 24
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -parent.width/2 - 20
        source: root.source_v
        rotation: root.rotation
    }

    Text{
        id: text
        anchors.verticalCenter: parent.verticalCenter
        textFormat: Text.RichText
        color: root.text_color
        text: root.contentText
        font.pixelSize: 25
        font.family: button_home.name
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        NumberAnimation on scale{duration: 200; easing.type: Easing.Linear}
        scale: root.pressed ? 0.8 : 1
    }

    MultiEffect {
        id: effect
        anchors.fill: image
        source: image
        shadowEnabled: true
        shadowColor: "black"
        shadowOpacity: 1
        shadowBlur: 0.5
        shadowHorizontalOffset: 5
        shadowVerticalOffset: 5
        NumberAnimation on scale{duration: 200; easing.type: Easing.Linear}
        scale: root.pressed ? 0.8 : 1
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
        NumberAnimation on scale{duration: 200; easing.type: Easing.Linear}
        scale: root.pressed ? 0.8 : 1
    }
}
