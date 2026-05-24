import QtQuick 2.15

Item {
    id: root
    width: 420
    height: 140
    clip: true

    property color windColor: "#280092ff"
    property real strandCount: 7.0
    property real speed: 1.0
    property real spread: 8.5
    property real ampMain: 13.0
    property real ampDetail: 4.2
    property real thickness: 2.0
    property real glowScale: 4.5
    property real brightness: 1.15

    property real timeValue: 0.0
    property real flowValue: 0.0

    NumberAnimation on timeValue {
           from: 0
           to: 1000
           duration: Math.max(1, 120000 / Math.max(0.1, root.speed))
           loops: Animation.Infinite
           running: true
       }

       NumberAnimation on flowValue {
           from: 0
           to: 1
           duration: Math.max(1, 1800 / Math.max(0.1, root.speed))
           loops: Animation.Infinite
           running: true
       }

    ShaderEffect {
        anchors.fill: parent

        blending: true
        fragmentShader: "qrc:/windflow.frag.qsb"

        property color windColor: root.windColor
        property size iResolution: Qt.size(width, height)
        property real time: root.timeValue
        property real flow: root.flowValue
        property real strandCount: root.strandCount
        property real spread: root.spread
        property real ampMain: root.ampMain
        property real ampDetail: root.ampDetail
        property real thickness: root.thickness
        property real glowScale: root.glowScale
        property real brightness: root.brightness
    }
}
