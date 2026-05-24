import QtQuick
import QtQuick.Effects
import Qt5Compat.GraphicalEffects

Rectangle{
    id: player; width: 400; height: 310; color: "#3f6f6f6f"; radius: 10 

    property string title: ""
    property string album: ""
    property string artist: ""
    property string image_s: ""

    property bool av_connected: false

    property int duration: 0
    property int position: 0

    property string status: ""

    function time2str(value) {
            return Qt.formatTime(new Date(value), (value > 3600000) ? 'hh:mm:ss' : 'mm:ss')
        }

    Rectangle{
        id: mask; anchors.fill: parent; radius: 10; color: "transparent"; visible: player.av_connected
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
    }

    Image{
        anchors.fill: parent; source: "images/music.png"; visible: !player.av_connected; opacity: 0.7
    }

    BorderColor{
    }
    Rectangle {
        anchors.fill: parent
        scale: 1.01
        radius: 10
        color: "transparent"
        border.width: 3
        layer.enabled: true
        layer.effect: MultiEffect {
            blurEnabled: true
            blur:0.6
        }
        BorderColor{borderWidth: 2}
    }

    Text {
        anchors.centerIn: parent; text: "Hãy kết nối bluetooth"; color: "#B6B6B6"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        font.pixelSize: 20; font.family: fontLoader.name; elide: Text.ElideMiddle; visible: !player.av_connected
    }

    Item {
        width: 100; height: 100; anchors.horizontalCenter: parent.horizontalCenter; y: 30; visible: player.av_connected
        property alias source: img.source
        Rectangle {
            id: maskk
            width: parent.width; height: parent.height; color: "black"; radius: parent.width/2; anchors.horizontalCenter: parent.horizontalCenter
            Image {
                id: img
                anchors.fill: parent
                source: "images/blue.jpg"
                layer.enabled: true
                layer.effect: OpacityMask {
                    maskSource: maskk
                }
                Rectangle{
                    anchors.centerIn: parent; width: 20; height: 20; radius: 10; color: "black"
                }
                NumberAnimation on rotation{loops: Animation.Infinite; from: 0; to: 360; duration: 10000; easing.type: Easing.Linear}
            }
        }
        MultiEffect {
            id: effect_text
            anchors.fill: mask
            source: mask
            shadowEnabled: true
            shadowColor: "black"
            shadowOpacity: 1
            shadowBlur: 0.6
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 4
        }
    }

    Text {
        id: title
        width: 300; anchors.horizontalCenter: parent.horizontalCenter; y: 150; text: player.title; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        font.pixelSize: 30; font.family: fontLoader.name; elide: Text.ElideMiddle; visible: player.av_connected
        // MultiEffect {
        //     anchors.fill: parent; source: parent; shadowEnabled: true; shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.6; shadowHorizontalOffset: 0; shadowVerticalOffset: 4
        // }
    }

    Text {
        id: artis
        width: 300; anchors.horizontalCenter: parent.horizontalCenter; y: 190; text: player.artist; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        font.pixelSize: 18; font.family: fontLoader.name; elide: Text.ElideMiddle; visible: player.av_connected
        // MultiEffect {
        //     anchors.fill: parent; source: parent; shadowEnabled: true; shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.6; shadowHorizontalOffset: 0; shadowVerticalOffset: 4
        // }
    }

    Row{
        id: control_media; width: 190; height: 50; spacing: 20; anchors.horizontalCenter: parent.horizontalCenter; y: 230; visible: player.av_connected
        ButtonControl{width: 50; height: 50; source: "images/previous.png"; width_image: 40; height_image: 40
            onClicked:{mediaplayer.previous()}
        }
        ButtonControl{width: 50; height: 50; source: player.status == "playing" ? "images/pause.png" : "images/play.png"; width_image: 40; height_image: 40; visible: player.av_connected
            onClicked:{player.status == "playing" ? mediaplayer.pause() : mediaplayer.play()}
        }
        ButtonControl{width: 50; height: 50; source: "images/next.png"; width_image: 40; height_image: 40; visible: player.av_connected
            onClicked:{mediaplayer.next()}
        }
    }

    Image{
        x: 100; y: 100; width: 100; height: 100
        source: player.image_s
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
                if ('image' in track) player.image_s = track.image
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

    // Connections {
    //     target: VehicleSignals

    //     onConnected: {
	//     VehicleSignals.authorize()
    //     }

    //     onAuthorized: {
	//     VehicleSignals.subscribe("Vehicle.Cabin.SteeringWheel.Switches.Next")
	//     VehicleSignals.subscribe("Vehicle.Cabin.SteeringWheel.Switches.Previous")
	//     VehicleSignals.subscribe("Vehicle.Cabin.SteeringWheel.Switches.Mode")
	// }

    //     onSignalNotification: {
    //         if (path === "Vehicle.Cabin.SteeringWheel.Switches.Next" && value === "true") {
    //             mediaplayer.next()
    //         } else if (path === "Vehicle.Cabin.SteeringWheel.Switches.Previous" && value === "true") {
    //             mediaplayer.previous()
    //         } else if (path === "Vehicle.Cabin.SteeringWheel.Switches.Mode" && value === "true") {
    //             if (player.av_connected)
    //                 mediaplayer.connect()
    //             else
    //                 mediaplayer.disconnect()
    //         }
    //     }
    // }

     Component.onCompleted : {
        mediaplayer.start()
        //VehicleSignals.connect()
    }

}