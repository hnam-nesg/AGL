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

    FontLoader{
        id: button_home
        source: ASSET_BASE + "Roboto-BoldCondensed.ttf"
    }
    enabled: true
    checkable: true
    background: Rectangle{
        id: rect_
        visible: true
        anchors.fill: parent
        color: "transparent"
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
        //anchors.fill: parent
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
        width: 10; height: 10
        text: root.text_sub
        anchors.horizontalCenter: image.horizontalCenter
        anchors.verticalCenter: image.verticalCenter
        font.pixelSize: 20
        font.family: button_home.name
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.horizontalCenterOffset: root.text_direct
    }

    Image{
        width: 60; height: 4
        anchors.horizontalCenter: image.horizontalCenter
        anchors.verticalCenter: image.verticalCenter
        anchors.verticalCenterOffset: 35
        source: root.source_
    }

    Image{
        id: image_v
        width: 50; height: 4
        anchors.horizontalCenter: image.horizontalCenter
        anchors.verticalCenter: image.verticalCenter
        anchors.horizontalCenterOffset: -50
        source: root.source_v
        rotation: root.rotation
    }

    Image{
        width: 35; height: 35
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenterOffset: -45
        source: root.source__
    }

    Text{
        id: text
        anchors.fill: parent
        textFormat: Text.RichText
        color: root.text_color
        text: root.contentText
        font.pixelSize: root.fontSize
        font.family: button_home.name
        horizontalAlignment: root.align ? Text.AlignLeft : Text.AlignHCenter
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
        // SequentialAnimation on shadowHorizontalOffset {
        //     loops: Animation.Infinite
        //     NumberAnimation { to: 5; duration: 1500; easing.type: Easing.InOutSine }
        //     NumberAnimation { to: -5; duration: 1500; easing.type: Easing.InOutSine }
        // }
        // SequentialAnimation on shadowHorizontalOffset {
        //     loops: Animation.Infinite
        //     NumberAnimation { to: -5; duration: 1500; easing.type: Easing.InOutSine }
        //     NumberAnimation { to: 5; duration: 1500; easing.type: Easing.InOutSine }
        // }
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

