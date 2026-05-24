import QtQuick
import QtQuick.Effects
import QtQuick.Controls

ApplicationWindow {
    id: window
    width: 850
    height: 670
    color: "transparent"
    Rectangle{
        id: root; width: 860; height: 660; color: "#3f6f6f6f"; radius: 10
        Background{width: 1280; height: 800; anchors.right: parent.right}

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

        property string mode: "eco"

        Image{
            id: mode_car
            x: 300; y: 70; width: 550; height: 550; source: "images/drive-mode-" + root.mode + ".png"
        }

        Flickable {
            id: contentRoot
            width: 860; height: 660; clip: true; flickableDirection: Flickable.VerticalFlick

            contentWidth: width
            contentHeight: 950

            Text{
                x: 30; y: 30; font.family: fontLoader.name; font.pixelSize: 30; color: "white"
                text: "Cài đặt chế độ lái xe"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                MultiEffect {
                    anchors.fill: parent; source: parent; shadowEnabled: true
                    shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.5
                    shadowHorizontalOffset: 5; shadowVerticalOffset: 5
                }
            }

            Rectangle{
                x: 30; y: 90; width: 450; height: 100; color: "#3f6f6f6f"; radius: 20
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
                width: 150; height: 100; radius: 20; x:30; y: 90
                visible: eco.checked || normal.checked || sport.checked
            }

            Row{
                x: 30; y: 90; width: 450; height: 100; spacing: 0
                ButtonControl {
                    id: eco
                    checked: true; width: 150; height: 100; activeText: true; contentText: "ECO"; fontSize: 30
                    onClicked:{rect_energy_mode.x = 30; rect_energy_mode.y = 90; normal.checked = true; sport.checked = true; root.mode = "eco"}
                }
                ButtonControl {
                    id: normal
                    width: 150; height: 100; activeText: true; contentText: "NORMAL"; fontSize: 30
                    onClicked:{rect_energy_mode.x = 180; rect_energy_mode.y = 90; eco.checked = true; sport.checked = true; root.mode = "normal"}
                }
                ButtonControl {
                    id: sport
                    width: 150; height: 100; activeText: true; contentText: "SPORT"; fontSize: 30
                    onClicked:{rect_energy_mode.x = 330; rect_energy_mode.y = 90; normal.checked = true; eco.checked = true; root.mode = "sport"}
                }
            }

            Image{
                x: 30; y: 210;width: 500; height: 2; source: "images/line_devider.png"
            }


            Text{
                x: 30; y: 230; font.family: fontLoader.name; font.pixelSize: 20; color: "white"
                text: "Hệ thống cân bằng điện tử (ESC)"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }

            Rectangle{
                x: 30; y: 270; width: 300; height: 100; color: "#3f6f6f6f"; radius: 15
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
                id: rect_esc
                width: 150; height: 100; radius: 15
                visible: esc_on.checked || esc_off.checked
            }

            Row{
                x: 30; y: 270; width: 300; height: 100; spacing: 0
                ButtonControl {
                    id: esc_on
                    width: 150; height: 100; source: "images/mirror-left.png"; width_image: 60; height_image: 60
                    onClicked:{rect_esc.x = 30; rect_esc.y = 270; esc_off.checked = true}
                }
                ButtonControl {
                    id: esc_off
                    width: 150; height: 100; source: "images/mirror-right.png"; width_image: 60; height_image: 60
                    onClicked:{rect_esc.x = 180; rect_esc.y = 270; esc_on.checked = true}
                }
            }

            Image{
                x: 30; y: 390;width: 500; height: 2; source: "images/line_devider.png"
            }

            Text{
                x: 30; y: 410; font.family: fontLoader.name; font.pixelSize: 20; color: "white"
                text: "Hệ thống kiểm soát lực kéo (TCS)"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }

            Rectangle{
                x: 30; y: 450; width: 200; height: 60; color: "#3f6f6f6f"; radius: 15
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
                id: rect_tcs
                width: 100; height: 60; radius: 15
                visible: tcs_on.checked || tcs_off.checked
            }

            Row{
                x: 30; y: 450; width: 200; height: 60; spacing: 0
                ButtonControl {
                    id: tcs_on
                    width: 100; height: 60; activeText: true; contentText: "Tắt"; fontSize: 20
                    onClicked:{rect_tcs.x = 30; rect_tcs.y = 450; tcs_off.checked = true}
                }
                ButtonControl {
                    id: tcs_off
                    width: 100; height: 60; activeText: true; contentText: "Bật"; fontSize: 20
                    onClicked:{rect_tcs.x = 130; rect_tcs.y = 450; tcs_on.checked = true}
                }
            }

            Image{
                x: 30; y: 530;width: 500; height: 2; source: "images/line_devider.png"
            }

            Text{
                x: 30; y: 550; font.family: fontLoader.name; font.pixelSize: 20; color: "white"
                text: "Chế độ di chuyển chậm"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }

            Rectangle{
                x: 30; y: 590; width: 200; height: 60; color: "#3f6f6f6f"; radius: 15
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
                id: rect_move
                width: 100; height: 60; radius: 15
                visible: slow_on.checked || slow_off.checked
            }

            Row{
                x: 30; y: 590; width: 200; height: 60; spacing: 0
                ButtonControl {
                    id: slow_on
                    width: 100; height: 60; activeText: true; contentText: "Tắt"; fontSize: 20
                    onClicked:{rect_move.x = 30; rect_move.y = 590; slow_on.checked = true}
                }
                ButtonControl {
                    id: slow_off
                    width: 100; height: 60; activeText: true; contentText: "Bật"; fontSize: 20
                    onClicked:{rect_move.x = 130; rect_move.y = 590; slow_off.checked = true}
                }
            }

            Image{
                x: 30; y: 670;width: 500; height: 2; source: "images/line_devider.png"
            }

            Text{
                x: 30; y: 690; font.family: fontLoader.name; font.pixelSize: 20; color: "white"
                text: "Tự động chuyển số P"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }

            Rectangle{
                x: 30; y: 730; width: 200; height: 60; color: "#3f6f6f6f"; radius: 15
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
                id: rect_p
                width: 100; height: 60; radius: 15
                visible: p_off.checked || p_on.checked
            }

            Row{
                x: 30; y: 730; width: 200; height: 60; spacing: 0
                ButtonControl {
                    id: p_off
                    width: 100; height: 60; activeText: true; contentText: "Tắt"; fontSize: 20
                    onClicked:{rect_p.x = 30; rect_p.y = 730; p_on.checked = true}
                }
                ButtonControl {
                    id: p_on
                    width: 100; height: 60; activeText: true; contentText: "Bật"; fontSize: 20
                    onClicked:{rect_p.x = 130; rect_p.y = 730; p_off.checked = true}
                }
            }

            Image{
                x: 30; y: 810;width: 500; height: 2; source: "images/line_devider.png"
            }

            Text{
                x: 30; y: 830; font.family: fontLoader.name; font.pixelSize: 20; color: "white"
                text: "Phanh tái sinh"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }

            Rectangle{
                x: 30; y: 870; width: 300; height: 60; color: "#3f6f6f6f"; radius: 15
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
                id: rect_brake
                width: 100; height: 60; radius: 15
                visible: low.checked || medium.checked || high.checked
            }

            Row{
                x: 30; y: 870; width: 300; height: 60; spacing: 0
                ButtonControl {
                    id: low
                    width: 100; height: 60; activeText: true; contentText: "Thấp"; fontSize: 20
                    onClicked:{rect_brake.x = 30; rect_brake.y = 870; medium.checked = true; high.checked = true}
                }
                ButtonControl {
                    id: medium
                    width: 100; height: 60; activeText: true; contentText: "Trung bình"; fontSize: 20
                    onClicked:{rect_brake.x = 130; rect_brake.y = 870; low.checked = true; high.checked = true}
                }
                ButtonControl {
                    id: high
                    width: 100; height: 60; activeText: true; contentText: "Cao"; fontSize: 20
                    onClicked:{rect_brake.x = 230; rect_brake.y = 870; low.checked = true; medium.checked = true}
                }
            }
        }
    }
}

