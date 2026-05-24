import QtQuick
import ClusterSplash 1.0

Item {
    id: root
    clip: true

    property string panelSource: "qrc:/asset/Panel.png"
    property string frameSource: "qrc:/asset/Panel_2.png"
    property string logoSource: "qrc:/asset/suzuki_logo.png"

    readonly property real baseWidth: 1280
    readonly property real baseHeight: 800
    readonly property real sx: width / baseWidth
    readonly property real sy: height / baseHeight

    Rectangle {
        anchors.fill: parent
        color: "#000000"
        z: 0
    }

    Image {
        id: panelImage
        source: root.panelSource

        x: 10 * root.sx
        y: -25 * root.sy
        width: 1260 * root.sx
        height: 800 * root.sy

        fillMode: Image.Stretch
        smooth: true
        mipmap: true
        asynchronous: false
        z: 10
    }

    ShimmerLogo {
        id: shimmerLogo

        readonly property real maxLogoWidth: Math.min(root.width * 0.58, 560 * root.sx)
        readonly property real maxLogoHeight: Math.min(root.height * 0.63, 500 * root.sy)

        width: maxLogoWidth
        height: maxLogoHeight

        x: root.width / 2 - width / 2
        y: root.height / 2 - root.height / 35 - height / 2

        source: root.logoSource
        running: root.visible
        fps: 30
        logoOpacity: 0.96
        shimmerSpeed: 0.42

        z: 30
    }

    Image {
        id: frameImage
        source: root.frameSource

        x: -180 * root.sx
        y: -130 * root.sy
        width: 1640 * root.sx
        height: 1060 * root.sy

        fillMode: Image.Stretch
        smooth: true
        mipmap: true
        asynchronous: false
        z: 100
    }
}