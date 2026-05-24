import QtQuick

Item {
    id: root
    clip: true

    property real frame: 0
    readonly property real fadeFrames: 36
    readonly property real sweepStart: 18
    readonly property real sweepEnd: 66
    readonly property real fadeProgress: Math.min(frame / fadeFrames, 1)
    readonly property real logoEase: 1 - Math.pow(1 - fadeProgress, 2)
    readonly property real logoScale: 0.94 + 0.06 * logoEase
    readonly property real sweepProgress: Math.max(0, Math.min((frame - sweepStart) / (sweepEnd - sweepStart), 1))
    readonly property bool sweepVisible: frame >= sweepStart && frame <= sweepEnd

    Rectangle {
        anchors.fill: parent
        color: "#000000"
    }

    Image {
        id: logo
        x: root.width / 2 - width / 2
        y: root.height / 2 - height / 2
        width: Math.max(1, implicitWidth * root.logoScale)
        height: Math.max(1, implicitHeight * root.logoScale)
        source: "file:///usr/share/plymouth/themes/mytheme/logo.png"
        fillMode: Image.PreserveAspectFit
        opacity: root.fadeProgress
        smooth: true
        mipmap: true
    }

    Image {
        id: glow
        width: implicitWidth
        height: implicitHeight
        source: "file:///usr/share/plymouth/themes/mytheme/glow.png"
        x: logo.x - width + (logo.width + width * 2) * root.sweepProgress
        y: logo.y + logo.height / 2 - height / 2
        opacity: root.sweepVisible
                 ? Math.max(0, 0.88 * (1 - Math.pow(root.sweepProgress - 0.5, 2) * 4) * root.fadeProgress)
                 : 0
        smooth: true
        mipmap: true
    }

    NumberAnimation on frame {
        from: 0
        to: 66
        duration: 1100
        running: true
    }
}
