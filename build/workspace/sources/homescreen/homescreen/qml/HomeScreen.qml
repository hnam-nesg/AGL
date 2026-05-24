import QtQuick
import QtQuick3D
import QtQuick.Controls.Material
import QtQuick.Effects
import QtQuick3D.Helpers

Item {
    id: root
    x: 80; y: 50; width: 1200; height: 670

    FontLoader{
        id: fontLoader
        source: "images/Roboto-BoldCondensed.ttf"
    }
    Rectangle {
        id: stateCar
        width: 350; height: 670; color: "transparent"
        SwipeView {
            id: swipe
            anchors.fill: parent
            interactive: true; clip: true
            Page {
                background: null
                Image{
                    anchors.fill: parent; source: "images/mercedes_normal.png"
                }
                View3D {
                    anchors.fill: parent
                    environment: SceneEnvironment {
                        backgroundMode: SceneEnvironment.Transparent
                        clearColor: "transparent"
                        lightProbe: Texture {
                            source: "images/qwantani_mid_morning_puresky_1k.hdr"
                        }
                        antialiasingMode: SceneEnvironment.MSAA
                    }

                    Mercedes {id: mercedes; scale: Qt.vector3d(1, 1, 1) }
                }

                LeaderCallout {
                    id: door
                    property string mode_door: "ĐANG KHÓA"
                    anchor: Qt.point(100, 320)
                    elbow:  Qt.point(70, 290)
                    labelPos: Qt.point(70, 230)
                    text: mode_door + "<br/><span style='color: #acacac; font-size: 15px;'>Cửa xe</span>"
                    y_: -45; x_: -10
                    onChecked_Changed: {Handler3D.door = checked_; mode_door = Handler3D.door ? "ĐÃ MỞ KHÓA" : "ĐANG KHÓA"}
                }

                LeaderCallout {
                    id: capo
                    property string mode_capo: "MỞ"
                    anchor: Qt.point(220, 250)
                    elbow:  Qt.point(220, 220)
                    labelPos: Qt.point(260, 220)
                    text: mode_capo + "<br/><span style='color: #acacac; font-size: 15px;'>Nắp capo</span>"
                    x_: 10; y_: -10
                    onChecked_Changed: {Handler3D.capo = checked_; mode_capo = Handler3D.capo ? "ĐÓNG" : "MỞ"}
                }

                LeaderCallout {
                    id: trunk
                    property string mode_trunk: "MỞ"
                    anchor: Qt.point(175, 565)
                    elbow:  Qt.point(175, 590)
                    labelPos: Qt.point(80, 590)
                    text: mode_trunk + "<br/><span style='color: #acacac; font-size: 15px;'>Cốp sau</span>"
                    x_: -60; y_: -5
                    onChecked_Changed: {Handler3D.trunk = checked_; mode_trunk = Handler3D.trunk ? "ĐÓNG" : "MỞ"; indicator_ani_l.restart(); indicator_ani_r.restart()}
                }

                Button {
                    id: button_RL
                    x: checked ? 20 : 90; y: 420; width: checked ? 100 : 50; height: checked ? 50 : 80
                    opacity: 0; checkable: true; enabled: Handler3D.door; rotation: checked ? -45 : 0
                    onClicked:{mercedes.doorRL.eulerRotation.z = checked ? -45 : 0}
                }

                Button {
                    id: button_FL
                    x: checked ? 20 : 90; y: 340; width: checked ? 100 : 50; height: checked ? 50 : 70
                    opacity: 0; checkable: true; enabled: Handler3D.door; rotation: checked ? -45 : 0
                    onClicked:{mercedes.doorFL.eulerRotation.z = checked ? -45 : 0}
                }

                Button {
                    id: button_FR
                    x: checked ? 215 : 210; y: 340; width: checked ? 100 : 50; height: checked ? 50 : 70
                    opacity: 0; checkable: true; enabled: Handler3D.door; rotation: checked ? -315 : 0
                    onClicked:{mercedes.doorFR.eulerRotation.z = checked ? 45 : 0}
                }

                Button {
                    id: button_RR
                    x: checked ? 215 : 210; y: 420; width: checked ? 100 : 50; height: checked ? 50 : 80
                    opacity: 0; checkable: true; enabled: Handler3D.door; rotation: checked ? -315 : 0
                    onClicked:{mercedes.doorRR.eulerRotation.z = checked ? 45 : 0}
                }
            }

            Page {
                background: null
                Image{
                    anchors.fill: parent; source: "images/mercedes_seat.png"
                }
                // View3D {
                //     anchors.fill: parent
                //     environment: SceneEnvironment  {
                //         backgroundMode: SceneEnvironment.Transparent
                //         clearColor: "transparent"
                //         lightProbe: Texture {
                //             source: "images/qwantani_mid_morning_puresky_1k.hdr"
                //         }

                //         probeOrientation: Qt.vector3d( 0,  -30,  -120)
                //         antialiasingMode: SceneEnvironment.MSAA
                //     }
                //     Mercedes_ { scale: Qt.vector3d(1, 1, 1) }
                // }
            }

            Page {
                background: null
                Image{
                    anchors.fill: parent; source: "images/mercedes_transparent.png"
                }
                // View3D {
                //     anchors.fill: parent
                //     environment: SceneEnvironment {
                //         backgroundMode: SceneEnvironment.Transparent
                //         clearColor: "transparent"
                //         lightProbe: Texture {
                //             source: "images/qwantani_mid_morning_puresky_1k.hdr"
                //         }
                //         //probeOrientation: Qt.vector3d( 0,  -30,  -120)
                //         antialiasingMode: SceneEnvironment.MSAA
                //     }
                //     Mercedes_Transparent { scale: Qt.vector3d(1, 1, 1) }
                // }
                Text {
                    id: tire_FL
                    x: 80; y: 340; text: "33 psi"; color: "#007a08"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 20; font.family: fontLoader.name
                }
                Text {
                    id: tire_FR
                    x: 220; y: 340; text: "34 psi"; color: "#007a08"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 20; font.family: fontLoader.name
                }
                Text {
                    id: tire_RL
                    x: 80; y: 460; text: "30 psi"; color: "#007a08"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 20; font.family: fontLoader.name
                }
                Text {
                    id: tire_RR
                    x: 220; y: 460; text: "31 psi"; color: "#007a08"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 20; font.family: fontLoader.name
                }
            }
        }

        PageIndicator {
            id: indicator
            y: 600
            anchors.horizontalCenter: swipe.horizontalCenter
            count: swipe.count
            currentIndex: swipe.currentIndex
            delegate: Rectangle {
                height: 8
                width: index === indicator.currentIndex ? 24 : 8
                radius: height / 2

                color: index === indicator.currentIndex ? "#00E5FF" : "#FFFFFF"
                opacity: index === indicator.currentIndex ? 1.0 : 0.25

                Behavior on width { NumberAnimation { duration: 200 } }
                Behavior on opacity { NumberAnimation { duration: 200 } }
                Rectangle {
                    anchors.fill: parent
                    scale: 1.1
                    color: "white"
                    radius: 4
                    layer.enabled: true
                    layer.effect: MultiEffect {
                        blurEnabled: true
                        blur:0.6
                    }
                    BorderColor{borderWidth: 10; c1: "#00E5FF"; c2: "#00E5FF"; radius: 4}
                }
            }
        }

        Image{
            id: headlight
            x: 15; y: 10; width: 35; height: 35; source: "images/high-beam.png"
        }

        Image{
            id: indicator_l
            x: 70; width: 30; height: 50;anchors.verticalCenter: mode_car.verticalCenter; source: "images/indicator_left.png"
            SequentialAnimation on opacity {
                id: indicator_ani_l
                loops: 3
                OpacityAnimator {from: 0; to: 1; duration: 500; easing.type: Easing.Linear }
                OpacityAnimator {from: 1; to: 0; duration: 500; easing.type: Easing.Linear }
            }
        }
        Text{
            id: mode_car
            anchors.horizontalCenter: parent.horizontalCenter; font.family: fontLoader.name; font.pixelSize: 25; color: "#0bb600"
            text: "READY"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        }
        Text{
            id: mode_gear
            anchors.horizontalCenter: parent.horizontalCenter; anchors.top: mode_car.bottom; font.family: fontLoader.name; font.pixelSize: 80; color: "white"
            text: "P"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        }
        Image{
            id: indicator_r
            x: 250; width: 30; height: 50;anchors.verticalCenter: mode_car.verticalCenter; source: "images/indicator_right.png"
            SequentialAnimation on opacity {
                id: indicator_ani_r
                loops: 3
                OpacityAnimator {from: 0; to: 1; duration: 500; easing.type: Easing.Linear }
                OpacityAnimator {from: 1; to: 0; duration: 500; easing.type: Easing.Linear }
            }
        }
        Image{
            id: warning_handbrake
            x: 300; width: 35; height: 35; source: "images/parking-brake.png"
        }
        Image{
            id: warning_seatbelt
            x: 300; y: 65; width: 35; height: 35; source: "images/warning-seatbelt.png"
        }
        Image{
            id: warning_seat
            x: 300; y: 115; width: 35; height: 50; source: "images/seat-driver.png"
        }
    }

    Row{
        width: 850; height: 640; anchors.left: stateCar.right; anchors.verticalCenter: parent.verticalCenter; spacing: 10
        AreaMap{
            MouseArea{
                    anchors.fill: parent
                    onClicked:{homescreenHandler.tapShortcut("navigation")}
            }
        }

        Column{
            width: 400; height:parent.height; spacing: 20
            MediaAreaMusic{
                MouseArea{
                    width: parent.width; height: 200
                    onClicked:{homescreenHandler.tapShortcut("mediaplayer")}
                }
            }
        }
    }
}