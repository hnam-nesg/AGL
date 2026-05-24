import QtQuick
import QtQuick.Effects
import QtQuick.Controls

ApplicationWindow{
    id: root
    width: 850
    height: 670
    color: "transparent"

    Rectangle{
        Background{width: 1280; height: 800; anchors.right: parent.right}

        FontLoader{
            id: fontLoader
            source: "images/Roboto-BoldCondensed.ttf"
        }

        width: 860; height: 660//; color: "#3f6f6f6f"; radius: 10

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

        Image{
            x: 260; y: 30; width: 600; height: 600; source: "images/background.png"
        }

        Text{
            x: 30; y: 30; font.family: fontLoader.name; font.pixelSize: 30; color: "white"
            text: "Điều chỉnh vô lăng"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            MultiEffect {
                anchors.fill: parent; source: parent; shadowEnabled: true
                shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.5
                shadowHorizontalOffset: 5; shadowVerticalOffset: 5
            }
        }

        Rectangle{
            x: 30; y: 530; width: 150; height: 100; color: "#3f6f6f6f"; radius: 20
            BorderColor{radius: 20}
            Rectangle {
                anchors.fill: parent
                scale: 1.01
                radius: 20
                color: "transparent"
                border.width: 3
                layer.enabled: true
                layer.effect: MultiEffect {
                    blurEnabled: true
                    blur:0.6
                }
                BorderColor{borderWidth: 2; radius: 20}
            }
        }

        HL_Button{
            id: rect_energy_mode
            width: 150; height: 100; radius: 20; x:30; y: 530
            visible: eco.checked
        }


        ButtonControl {
            id: eco
            x: 30; y: 530; width: 150; height: 100; activeText: true; contentText: "Bật sưởi"; fontSize: 30
        }
    }
}

