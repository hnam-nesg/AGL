import QtQuick
import QtQuick.Effects
Item {
    width: 130; height: 50; x: 30
    property alias horizontalAlignment: text.horizontalAlignment
    property alias verticalAlignment: text.verticalAlignment
    property alias wrapMode_: text.wrapMode
    property alias color: text.color
    property alias shadow: effect_text.shadowColor
    property alias effect_text: effect_text
    FontLoader{
        id: font_text
        source: "images/Roboto-BoldCondensed.ttf"
    }

    property alias contentText: text.text
    property alias fontSize: text.font.pixelSize

    Text {
        id: text
        width: parent.width; height: parent.height
        color: "#ffffff"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: font_text.name
        clip: true
    }
    MultiEffect {
        id: effect_text
        anchors.fill: text
        source: text
        shadowEnabled: true
        shadowColor: "black"
        shadowOpacity: 1
        shadowBlur: 0.5
        shadowHorizontalOffset: fontSize > 21 ? 5 : 2
        shadowVerticalOffset: fontSize > 21 ? 5 : 2
    }
}
