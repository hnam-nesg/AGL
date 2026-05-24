import QtQuick 
import QtQuick.Controls
import QtQuick.Effects
import Qt5Compat.GraphicalEffects


ApplicationWindow {
    id: root
    x: 420; y: 350;width: 400; height: 300
    flags: Qt.FramelessWindowHint
    visible: telephony.callState !== "disconnected"

    // Component.onCompleted: {
    //     telephony.autoSelectBluetoothDevice()
    // }
    property int callSeconds: 0
    property bool callAccepted: false
    property bool callInProgress: telephony.callState === "dialing"
                                  || telephony.callState === "alerting"
                                  || telephony.callState === "active"
                                  || telephony.callState === "held"
                                  || telephony.callState === "waiting"

    function formatTime(totalSeconds) {
        var mins = Math.floor(totalSeconds / 60)
        var secs = totalSeconds % 60
        var mm = mins < 10 ? "0" + mins : "" + mins
        var ss = secs < 10 ? "0" + secs : "" + secs
        return mm + ":" + ss
    }

    Timer {
        id: callTimer
        interval: 1000
        repeat: true
        running: false
        onTriggered: callSeconds += 1
    }


    FontLoader{
        id: fontLoader
        source: "images/Roboto-BoldCondensed.ttf"
    }

    Rectangle{
        id: mask; anchors.fill: parent; radius: 10
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

    // Image{
    //     x: 100; y: 20; width: 200; height: 120; source: "images/mammamia.jpg"
    // }

    Text{
        anchors.horizontalCenter: parent.horizontalCenter; y: 50; font.family: fontLoader.name; font.pixelSize:50; color: "white"
        text: telephony.callClip; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
    }

    Text{
        anchors.horizontalCenter: parent.horizontalCenter; y: 150; font.family: fontLoader.name; font.pixelSize:30; color: "white"
        text: formatTime(callSeconds); horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; visible: root.callInProgress
    }

    ButtonControl{
        id: call;x: 70; y: 200; width: 100; height: 100; width_image: 50; height_image: 50
        source: "images/phone-call.png"; incomingCall: telephony.callState === "incoming"; btn_call: "#39ff14"; visible: telephony.callState === "incoming"
        onClicked:{
            callAccepted = true; callSeconds = 0; callTimer.start(); telephony.answer()
        }
    }

    ButtonControl{
        id: reject;x: root.callInProgress ? 150 : 230; y: 200; width: 100; height: 100; width_image: 50; height_image: 50
        source: "images/call_reject.png"; incomingCall: telephony.callState === "incoming"; btn_call: "red"
        onClicked:{
            callTimer.stop(); callSeconds = 0; telephony.hangup()
        }
    }

    Connections {
        target: telephony

        function onCallStateChanged(state) {
            console.log("callState =", state)
            console.log("callClip =", telephony.callClip)

            if (state === "incoming") {
                callAccepted = false
                callTimer.stop()
                callSeconds = 0
            } else if (state === "dialing" || state === "alerting") {
                callAccepted = true
                callTimer.stop()
                callSeconds = 0
            } else if (state === "active" || state === "held") {
                callAccepted = true
                if (!callTimer.running)
                    callTimer.start()
            } else if (state === "disconnected") {
                callAccepted = false
                callTimer.stop()
                callSeconds = 0
            }
        }
    }
}
