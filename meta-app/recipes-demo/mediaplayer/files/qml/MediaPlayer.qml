import QtQuick
import QtQuick.Effects
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

ApplicationWindow{
    id: root
    width: 850; height: 670; color: "transparent"
    Rectangle{
        y: 5; width: 850; height: 660; color: "transparent"; radius: 10
        Background{width_:1280; height: 800; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter}
        FontLoader{
            id: fontLoader
            source: "images/Roboto-BoldCondensed.ttf"
        }

        Item {
            id: player

            property string title: ""
            property string album: ""
            property string artist: ""

            property bool av_connected: false

            property int duration: 0
            property int position: 0

            property string status: "stopped"

            function time2str(value) {
                return Qt.formatTime(new Date(value), (value > 3600000) ? 'hh:mm:ss' : 'mm:ss')
            }
        }

        Component.onCompleted : {
            mediaplayer.start()

            VehicleSignals.connect()
        }

        Connections {
            target: mediaplayer

            onMetadataChanged: {
                var track = metadata.track

                if ('status' in metadata) {
                    player.status = metadata.status
                }

                if ('connected' in metadata) {
                    player.av_connected = metadata.connected
                }

                if (track) {
                    if ('image' in track) return
                    player.title = track.title
                    player.album = track.album
                    player.artist = track.artist

                    if ('duration' in track)
                        player.duration = track.duration

                    if ('index' in track) {
                        playlistview.currentIndex = track.index
                    }
                }

                if ('position' in metadata) {
                    player.position = metadata.position
                }
            }
        }

        Connections {
            target: VehicleSignals

            onConnected: {
            VehicleSignals.authorize()
            }

            onAuthorized: {
            VehicleSignals.subscribe("Vehicle.Cabin.SteeringWheel.Switches.Next")
            VehicleSignals.subscribe("Vehicle.Cabin.SteeringWheel.Switches.Previous")
            VehicleSignals.subscribe("Vehicle.Cabin.SteeringWheel.Switches.Mode")
        }

            onSignalNotification: {
                if (path === "Vehicle.Cabin.SteeringWheel.Switches.Next" && value === "true") {
                    mediaplayer.next()
                } else if (path === "Vehicle.Cabin.SteeringWheel.Switches.Previous" && value === "true") {
                    mediaplayer.previous()
                } else if (path === "Vehicle.Cabin.SteeringWheel.Switches.Mode" && value === "true") {
                    if (player.av_connected)
                        mediaplayer.connect()
                    else
                        mediaplayer.disconnect()
                }
            }
        }

        Timer {
            id: timer
            interval: 250
            running: player.av_connected && player.status == "playing"
            repeat: true
            onTriggered: {
                player.position = player.position + 250
            }
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

        Slider {
            id: progressBar1
            y: 60; anchors.left: parent.left; anchors.leftMargin: 40; height: 200; width: 40
            enabled: true; orientation: Qt.Vertical; rotation: 180
            value: 0

            background: Rectangle {
                x: progressBar1.leftPadding + (progressBar1.availableWidth - width) / 2
                y: progressBar1.topPadding
                width: 5
                height: progressBar1.availableHeight
                radius: 2.5
                color: "gray"
                Rectangle {
                    width: 4; height: progressBar1.visualPosition * parent.height; color: "white"; radius: 2
                }
            }
            handle: Rectangle {
                    id: point_center; width: 20; height: 20; radius: 10
                    color: "white"
                    x: progressBar1.leftPadding + (progressBar1.availableWidth - width) / 2
                    y: progressBar1.topPadding
                    + (1 -progressBar1.value) * (progressBar1.availableHeight - height)
                    visible:  false
                }
        }

        ButtonControl {
            id: volume
            anchors.top: progressBar1.bottom; anchors.horizontalCenter: progressBar1.horizontalCenter; width: 60; height: 60; source: "images/high-volume.png"; width_image: 30; height_image: 30
        }

        Rectangle{
            id: mask; y: 50; width: 400; height: 250; radius: 20; color: "transparent"; anchors.horizontalCenter: parent.horizontalCenter
            Image{
                id: image_blur
                anchors.fill: parent; source: "images/blue.jpg"; layer.enabled: true
                layer.effect: MultiEffect{
                    blur: 0.6
                    blurEnabled: true
                    opacity: 0.5
                    shadowBlur: 0.5
                    shadowHorizontalOffset: 5
                    shadowVerticalOffset: 5
                }
                OpacityMask {
                    maskSource: mask
                }
                fillMode: Image.PreserveAspectCrop
            }
            Image{
                width: 200; height: 150; anchors.centerIn: parent; source: "images/blue.jpg"
            }
        }

        Text {
            id: title
            width: 600; anchors.horizontalCenter: parent.horizontalCenter; y: 330; text: player.title; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            font.pixelSize: 30; font.family: fontLoader.name;  elide: Text.ElideRight
            // MultiEffect {
            //     anchors.fill: parent; source: parent; shadowEnabled: true; shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.6; shadowHorizontalOffset: 0; shadowVerticalOffset: 4
            // }
        }

        Text {
            id: artis
            width: 600; anchors.horizontalCenter: parent.horizontalCenter; y: 370; text: player.artist; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            font.pixelSize:18; font.family: fontLoader.name; elide: Text.ElideRight
            // MultiEffect {
            //     anchors.fill: parent; source: parent; shadowEnabled: true; shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.6; shadowHorizontalOffset: 0; shadowVerticalOffset: 4
            // }
        }

        Slider{
            id: progressBar
            y: 450; anchors.horizontalCenter: parent.horizontalCenter
            width: 700
            enabled: true
            value: player.position
            background: Rectangle {
                x: progressBar.leftPadding
                y: progressBar.topPadding + progressBar.availableHeight / 2 - height / 2
                implicitWidth: 200
                implicitHeight: 4
                width: progressBar.availableWidth
                height: implicitHeight
                radius: 2
                color: "gray"

                Rectangle {
                    width: progressBar.visualPosition * parent.width
                    height: parent.height
                    color: "white"
                    radius: 2
                }
            }

            Label {
                id: position
                anchors.left: parent.left; anchors.bottom: parent.top;anchors.bottomMargin: 10; anchors.leftMargin: -20; font.family: fontLoader.name; font.pixelSize: 25
                text: player.time2str(player.position)
            }
            Label {
                id: duration
                anchors.right: parent.right; anchors.bottom: parent.top; anchors.bottomMargin: 10; anchors.rightMargin: -20; font.family: fontLoader.name; font.pixelSize: 25
                text: player.time2str(player.duration)
            }

            handle: Image {
                anchors.verticalCenter: parent.verticalCenter
                x: progressBar.leftPadding + progressBar.visualPosition * (progressBar.availableWidth - width)
                y: progressBar.topPadding + progressBar.availableHeight / 2 - height / 2
                //source: "images/center_point.png"
                Image {
                    anchors.centerIn: parent
                    source: "images/center_point.png"
                }
            }
            onMoved: {
                player.position = player.duration * progressBar.position
            }
        }

        Row{
            y: 500; width: 340; height: 100; anchors.horizontalCenter: parent.horizontalCenter; spacing: 20
            ButtonControl {
                id: previous
                width: 100; height: 100; source: "images/previous_.png"; width_image: 60; height_image: 60
            }
            ButtonControl {
                id: play
                width: 100; height: 100; source: "images/pause_.png"; width_image: 100; height_image: 100
            }
            ButtonControl {
                id: next
                width: 100; height: 100; source: "images/next_.png"; width_image: 60; height_image: 60
            }
        }
    }
}
