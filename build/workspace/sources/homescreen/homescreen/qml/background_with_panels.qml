import QtQuick
import QtQuick.Window

Window {
    id: window
    visibility: Window.FullScreen
    flags: Qt.FramelessWindowHint
    visible: true
    color: "#000000"

    property bool shellContextReady: false
    property bool splashEffectDone: false
    property bool shellLoaded: false
    readonly property bool shellUiReady: shellLoaded
                                        && shellContent.item
                                        && shellContent.item.uiReady
    readonly property bool shellReadyToShow: splashEffectDone && shellUiReady

    StartupSplash {
        anchors.fill: parent
        z: 100
        shimmerPasses: 2
        shimmerSpeed: 0.5
        opacity: window.shellReadyToShow ? 0 : 1
        visible: opacity > 0

        onFinished: window.splashEffectDone = true

        Behavior on opacity {
            NumberAnimation { duration: 180 }
        }
    }

    Loader {
        id: shellContent
        anchors.fill: parent
        active: false
        asynchronous: true
        visible: active
        opacity: window.shellReadyToShow ? 1 : 0
        source: "ShellContent.qml"

        onLoaded: {
            window.shellLoaded = true
        }

        Behavior on opacity {
            NumberAnimation { duration: 180 }
        }
    }

    Binding {
        target: shellContent
        property: "active"
        value: window.shellContextReady && window.splashEffectDone
    }
}
