import QtQuick
import QtQuick.Effects
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

ApplicationWindow{
    width: 850; height: 670; color: "transparent"
    Rectangle{
        id: root
        Background{width: 1280; height: 800; anchors.right: parent.right}

        FontLoader{
            id: fontLoader
            source: "images/Roboto-BoldCondensed.ttf"
        }

        y: 5; width: 850; height: 660; color: "transparent"; radius: 10
        BorderColor{}
        Rectangle {
            anchors.fill: parent
            radius: 10
            color: "transparent"
            border.width: 3
            layer.enabled: true
            layer.effect: MultiEffect {
                blurEnabled: true
                blur:0.6
            }
            BorderColor{borderWidth: 1; c1: "white"; c2: "white"}
        }

        Text{
            x: 130; y: 30; font.family: fontLoader.name; font.pixelSize: 30; color: "white"
            text: menuList.model.get(menuList.currentIndex).name; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            MultiEffect {
                anchors.fill: parent; source: parent; shadowEnabled: true
                shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.5
                shadowHorizontalOffset: 5; shadowVerticalOffset: 5
            }
        }
        Image{
            x: -180; y: 370; width: 600; height: 2; rotation: 90; source: "images/line_devider.png"
        }

        ListModel {
            id: menuModel
            ListElement { name: "Cài đặt thiết bị"; source: "images/mobile_phone.png"}
            ListElement { name: "Cài đặt wifi"; source: "images/wifi.png"}
            ListElement { name: "Cài đặt trợ lí ảo"; source: "images/voice.png"}
            ListElement { name: "Cài đặt màn hình"; source: "images/screen.png"}
            ListElement { name: "Cài đặt âm thanh"; source: "images/volume.png"}
            ListElement { name: "Cài đặt xe"; source: "images/car_settings.png"}
            ListElement { name: "Cài đặt ánh sáng"; source: "images/light_settings.png"}
        }

        property int currentIndex: -1

        ListView {
            id: menuList
            y: 80; width: parent.width; height: parent.height - 80
            model: menuModel
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            delegate: ButtonControl {
                width: 120; height: 116; width_image: 40; height_image: 40; source: model.source
                onPressed:{
                    menuList.currentIndex = index
                    switch(index){
                        case 0:
                            pageSettings.source = "Device.qml"
                            break;
                        case 1:
                            pageSettings.source = "Wifi.qml"
                            break;
                        case 2:
                            pageSettings.source = "Assistant.qml"
                            break;
                        case 3:
                            pageSettings.source = "Display.qml"
                            break;
                        case 4:
                            pageSettings.source = "Sound.qml"
                            break;
                        case 5:
                            pageSettings.source = "Car.qml"
                            break;
                        case 6:
                            pageSettings.source = "Light.qml"
                            break;
                    }
                }
            }

            highlight: HL_Button {
                color: "#202a36"
                y: menuList.currentItem.y
                Behavior on y {NumberAnimation{duration: 300; easing.type: Easing.Linear }}
            }
            highlightMoveDuration: 300
            preferredHighlightBegin: height
            preferredHighlightEnd: 0

        }

        Rectangle {
            id: display_settings
            x: 120; y: 80; width: 760; height: 580; color: "transparent"
            Loader{
                id: pageSettings
                anchors.fill: parent
                asynchronous: true
                source: "Device.qml"
            }
        }
    }
}