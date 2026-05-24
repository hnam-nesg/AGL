import QtQuick
import QtQuick.Effects
import QtQuick3D
import QtQuick3D.Helpers

Window {
    id: root
    width: 850
    height: 670
    color: "transparent"
    Rectangle{
        y: 5; width: 860; height: 660; color: "#3f6f6f6f"; radius: 10

        Background{width_:1280; height: 800; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter}

        Image{
            x: 370; y: 30; width: 600; height: 600; source: "images/background.png"
        }

        FontLoader{
            id: fontLoader
            source: "images/Roboto-BoldCondensed.ttf"
        }

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
            x: 30; y: 30; font.family: fontLoader.name; font.pixelSize: 30; color: "white"
            text: "Gương chiếu hậu"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            MultiEffect {
                anchors.fill: parent; source: parent; shadowEnabled: true
                shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.5
                shadowHorizontalOffset: 5; shadowVerticalOffset: 5
            }
        }

        Rectangle{
            x: 30; y: 100; width: 300; height: 120; color: "#3f6f6f6f"; radius: 20
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

        Rectangle{
            x: 30; y: 240; width: 300; height: 60; color: "#3f6f6f6f"; radius: 15
            BorderColor{radius: 15}
            Rectangle {
                anchors.fill: parent
                scale: 1.01
                radius: 15
                color: "transparent"
                border.width: 3
                layer.enabled: true
                layer.effect: MultiEffect {
                    blurEnabled: true
                    blur:0.6
                }
                BorderColor{borderWidth: 2; radius: 15}
            }
        }

        HL_Button{
            id: rect_
            width: 150; height: 120; radius: 20
            visible: mirror_l.checked || mirror_r.checked
        }

        HL_Button{
            id: rect_mirror
            width: 300; height: 60; radius: 15
            visible: mirror_g.checked
        }

        Row{
            x: 30; y: 100; width: 300; height: 120; spacing: 0
            ButtonControl {
                id: mirror_l
                width: 150; height: 120; source: "images/mirror-left.png"; width_image: 60; height_image: 60
                onClicked:{rect_.x = 30; rect_.y = 100; mirror_r.checked = true}
            }
            ButtonControl {
                id: mirror_r
                width: 150; height: 120; source: "images/mirror-right.png"; width_image: 60; height_image: 60
                onClicked:{rect_.x = 180; rect_.y = 100; mirror_l.checked = true}
            }
        }

        ButtonControl {
            id: mirror_g
            x: 30; y: 240;width: 300; height: 60; activeText: true; contentText: "Gập gương"; fontSize: 24
            onClicked:{rect_mirror.x = 30; rect_mirror.y = 240}
        }

        Image{
            x: 30; y: 340;width: 300; height: 2; source: "images/line_devider.png"
        }

        Text{
            x: 30; y: 370; font.family: fontLoader.name; font.pixelSize: 20; color: "white"
            text: "Tự động gập khi khóa xe"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            // MultiEffect {
            //     anchors.fill: parent; source: parent; shadowEnabled: true
            //     shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.5
            //     shadowHorizontalOffset: 5; shadowVerticalOffset: 5
            // }
        }

        Rectangle{
            x: 30; y: 410; width: 200; height: 60; color: "#3f6f6f6f"; radius: 15
            BorderColor{radius: 15}
            Rectangle {
                anchors.fill: parent
                scale: 1.01
                radius: 15
                color: "transparent"
                border.width: 3
                layer.enabled: true
                layer.effect: MultiEffect {
                    blurEnabled: true
                    blur:0.6
                }
                BorderColor{borderWidth: 2; radius: 15}
            }
        }

        HL_Button{
            id: auto_mirror
            width: 100; height: 60; radius: 15
            visible: auto_mirror_off.checked || auto_mirror_on.checked
        }
        Row{
            x: 30; y: 410; width: 200; height: 60; spacing: 0
            ButtonControl {
                id: auto_mirror_off
                width: 100; height: 60; activeText: true; contentText: "Tắt"; fontSize: 20
                onClicked:{auto_mirror.x = 30; auto_mirror.y = 410; auto_mirror_on.checked = true}
            }
            ButtonControl {
                id: auto_mirror_on
                width: 100; height: 60; activeText: true; contentText: "Bật"; fontSize: 20
                onClicked:{auto_mirror.x = 130; auto_mirror.y = 410; auto_mirror_off.checked = true}
            }
        }

        Text{
            x: 30; y: 510; font.family: fontLoader.name; font.pixelSize: 20; color: "white"
            text: "Tự động nghiêng khi lùi"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            // MultiEffect {
            //     anchors.fill: parent; source: parent; shadowEnabled: true
            //     shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.5
            //     shadowHorizontalOffset: 5; shadowVerticalOffset: 5
            // }
        }

        Rectangle{
            x: 30; y: 550; width: 400; height: 60; color: "#3f6f6f6f"; radius: 15
            BorderColor{radius: 15}
            Rectangle {
                anchors.fill: parent
                scale: 1.01
                radius: 15
                color: "transparent"
                border.width: 3
                layer.enabled: true
                layer.effect: MultiEffect {
                    blurEnabled: true
                    blur:0.6
                }
                BorderColor{borderWidth: 2; radius: 15}
            }
        }
        HL_Button{
            id: auto_mirror_2
            width: 100; height: 60; radius: 15
            visible: off.checked || left.checked || right.checked || on.checked
        }
        Row{
            x: 30; y: 550; width: 400; height: 60; spacing: 0
            ButtonControl {
                id: off
                width: 100; height: 60; activeText: true; contentText: "Tắt"; fontSize: 20
                onClicked:{auto_mirror_2.x = 30; auto_mirror_2.y = 550; left.checked = true; right.checked = true; on.checked = true}
            }
            ButtonControl {
                id: left
                width: 100; height: 60; activeText: true; contentText: "Trái"; fontSize: 20
                onClicked:{auto_mirror_2.x = 130; auto_mirror_2.y = 550; off.checked = true; right.checked = true; on.checked = true}
            }
            ButtonControl {
                id: right
                width: 100; height: 60; activeText: true; contentText: "Phải"; fontSize: 20
                onClicked:{auto_mirror_2.x = 230; auto_mirror_2.y = 550; left.checked = true; off.checked = true; on.checked = true}
            }
            ButtonControl {
                id: on
                width: 100; height: 60; activeText: true; contentText: "Cả hai"; fontSize: 20
                onClicked:{auto_mirror_2.x = 330; auto_mirror_2.y = 550; left.checked = true; right.checked = true; off.checked = true}
            }
        }
    }
}

