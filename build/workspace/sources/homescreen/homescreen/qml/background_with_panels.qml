import QtQuick

Window {
    id: window
    visibility: Window.FullScreen
    flags: Qt.FramelessWindowHint
    color: "#091018"

    StartupSplash {
        anchors.fill: parent
        opacity: shellContent.visible ? 0 : 1
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation { duration: 180 }
        }
    }

    Loader {
        id: shellContent
        anchors.fill: parent
        active: false
        visible: false
        opacity: visible ? 1 : 0
        sourceComponent: shellComponent

        onLoaded: revealShell.start()

        Behavior on opacity {
            NumberAnimation { duration: 180 }
        }
    }

    Timer {
        interval: 150
        running: true
        repeat: false
        onTriggered: shellContent.active = true
    }

    Timer {
        id: revealShell
        interval: 80
        repeat: false
        onTriggered: shellContent.visible = true
    }

    Component {
        id: shellComponent

        Item {
            anchors.fill: parent

            Background {}

            HomeScreen { id: homescreen }

            TopBar { id: topbar }

            BottomBar {
                id: bottombar
                onModeBottom: {
                    leftbar.steering_checked = false; leftbar.light_checked = false; leftbar.mirror_checked = false; leftbar.glass_checked = false
                }
            }

            LeftBar {
                id: leftbar
                anchors.top: topbar.bottom
                onModeLeft: {
                    bottombar.drive_mode = "white"; bottombar.home = "images/home.png"; bottombar.menu = "images/menu.png"; bottombar.camera = "images/camera.png"; bottombar.fan = "images/fan.png"
                }
            }
        }
    }
}
