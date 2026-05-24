import QtQuick
import QtQuick.Effects
import QtQuick.Controls
import QtWebEngine

ApplicationWindow{
    id: root
    width: 850
    height: 670
    color: "transparent"

    Rectangle{
        Background{width: 1280; height: 800; anchors.right: parent.right}

        FontLoader{
            id: fontLoader
            source: "images/Roboto-BoldCondensed.ttf"
        }

        width: 860; height: 660//; color: "#3f6f6f6f"; radius: 10

        BorderColor{}

        WebEngineView {
            id: web
            anchors.fill: parent
            url: "https://www.youtube.com"
            settings.playbackRequiresUserGesture: false
        }
    }
}

