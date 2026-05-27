import QtQuick
import HomescreenSplash 1.0

Item {
    id: root
    clip: true

    property string logoSource: "qrc:/images/logo.png"
    property real fadeProgress: 0
    property int shimmerPasses: 2
    property real shimmerSpeed: 0.5
    property int fadeInDuration: 560
    property bool shimmerRunning: false

    signal finished()

    readonly property real logoEase: 1 - Math.pow(1 - fadeProgress, 2)
    readonly property real logoScale: 0.94 + 0.06 * logoEase

    Rectangle {
        anchors.fill: parent
        color: "#000000"
    }

    ShimmerLogo {
        id: logo
        x: root.width / 2 - width / 2
        y: root.height / 2 - height / 2
        width: Math.max(1, implicitWidth * root.logoScale)
        height: Math.max(1, implicitHeight * root.logoScale)
        source: root.logoSource
        running: root.visible && root.shimmerRunning
        fps: 30
        logoOpacity: 0.96
        shimmerSpeed: root.shimmerSpeed
        opacity: root.fadeProgress
    }

    Timer {
        interval: root.fadeInDuration
                  + Math.ceil((root.shimmerPasses / Math.max(root.shimmerSpeed, 0.02)) * 1000)
        running: true
        repeat: false
        onTriggered: {
            root.shimmerRunning = false
            root.finished()
        }
    }

    NumberAnimation on fadeProgress {
        from: 0
        to: 1
        duration: root.fadeInDuration
        easing.type: Easing.OutCubic
        running: true
        onStopped: root.shimmerRunning = true
    }
}
