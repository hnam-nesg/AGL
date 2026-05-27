import QtQuick

Item {
    id: shellRoot
    anchors.fill: parent

    property bool uiReady: homeLoader.status === Loader.Error
                           || (homeLoader.status === Loader.Ready
                               && homeLoader.item
                               && homeLoader.item.uiReady)

    Background {}

    Loader {
        id: homeLoader
        active: true
        asynchronous: true
        sourceComponent: homeScreenComponent
    }

    Component {
        id: homeScreenComponent

        HomeScreen {
            deferredContentActive: true
        }
    }

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
