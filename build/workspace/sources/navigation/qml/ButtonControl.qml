import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Effects
import Qt5Compat.GraphicalEffects
Button {
    id: root
    FontLoader{
        id: button_home
        source: "images/Roboto-BoldCondensed.ttf"
    }
    property int buttonWidth: 40
    property int buttonHeight: 40
    property int buttonx: 0
    property int buttony: 0
    property string source: ""
    property bool activeText: false
    property string contentText: ""
    property string source_fan: ""
    property string contentFan: ""
    property int fontSize: 10
    property bool fan: false
    property bool back: false
    property alias horizontalAlignment: text.horizontalAlignment
    property alias shadowColor: effect.shadowColor
    property alias background_: rect_.visible
    property alias color_bg: rect_.color
    property alias radius_bg: rect_.radius
    property alias image_width: image_fan.width
    property alias image_height: image_fan.height
    property alias colorText: text.color
    property alias shadow: effect.shadowColor
    scale: pressed ? 0.8 : 1
    NumberAnimation on scale{duration: 200; easing.type: Easing.Linear}
    width: root.buttonWidth
    height: root.buttonHeight
    x: root.buttonx
    y: root.buttony
    enabled: true
    checkable: false
    opacity: 1
    background: Rectangle{
        id: rect_
        visible: false
        anchors.fill: parent
        color: "#8aa7a7a7"
        scale: 0.98
        border.color: Qt.darker(color, 1.1)
        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 0; verticalOffset: 0; radius: 10
            samples: 20; color: "#80FFFFFF"
        }
    }
    Image{
        id: image
        anchors.fill: parent
        visible: !root.activeText && !root.fan
        source: root.source
        smooth: true
        fillMode: Image.PreserveAspectFit
    }

    Text{
        id: text
        anchors.fill: parent
        visible: root.activeText
        color: "white"
        text: root.contentText
        font.pixelSize: root.fontSize
        font.family: button_home.name
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    Row{
        id: fan
        x: 20
        //anchors.centerIn: parent
        anchors.verticalCenter: parent.verticalCenter
        visible: root.fan
        spacing: 10
        Image{
            id: image_fan
            anchors.verticalCenter: parent.verticalCenter
            height: 40
            width: 40
            source: root.source_fan
        }
        Text{
            id: text_fan
            anchors.verticalCenter: parent.verticalCenter
            color: "white"
            text: root.contentFan
            font.pixelSize: root.fontSize
            font.family: button_home.name
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    MultiEffect {
        id: effect
        anchors.fill: root.fan ? fan : image
        source: root.fan ? fan : image
        shadowEnabled: true
        shadowColor: "black"
        shadowOpacity: 1
        shadowBlur: 0.5
        shadowHorizontalOffset: 5
        shadowVerticalOffset: 5
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
}
