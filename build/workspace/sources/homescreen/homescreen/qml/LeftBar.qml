import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    id: root
    width: 80; height: 670
    signal modeLeft()
    property alias steering_checked: steering.checked
    property alias light_checked: light.checked
    property alias mirror_checked: mirror.checked
    property alias glass_checked: glass.checked

    Rectangle{
        anchors.fill: parent
        color: "black"
    }

    Rectangle{
        id: rect_
        visible: light.checked || mirror.checked || steering.checked || glass.checked
        width: 80; height: 59// color: "blue"
        // border.color: Qt.darker(color, 1.2)
        Canvas {
            anchors.fill: parent
            antialiasing: true

            onPaint: {
                const ctx = getContext("2d");
                ctx.reset();

                const g = ctx.createLinearGradient(0, 0, width, 0);
                g.addColorStop(0.3, "black");
                g.addColorStop(1.0, "#fa001b53");

                ctx.fillStyle = g;
                ctx.fillRect(0, 0, width, height);
            }
        }
        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 0; verticalOffset: 0
            samples: 20; color: "#80FFFFFF"
        }
    }

    Column{
        x: 15; y: 220
        spacing: 30

        ButtonControl {
            id: light
            width: 50; height: 50; source: "images/light.png"; source_: "images/line_devider.png"; width_image: 50; height_image: 50
            onClicked:{rect_.y = 220; steering.checked = true; mirror.checked = true; glass.checked = true; homescreenHandler.tapShortcut("light"); root.modeLeft()}
        }

        ButtonControl {
            id: mirror
            width: 50; height: 50; source: "images/mirror.png"; source_: "images/line_devider.png"; width_image: 50; height_image: 50
            onClicked:{rect_.y = 300; steering.checked = true; light.checked = true; glass.checked = true; homescreenHandler.tapShortcut("agl-mirror"); root.modeLeft()}
        }

        ButtonControl {
            id: steering
            width: 50; height: 50; source: "images/steering.png"; source_: "images/line_devider.png"; width_image: 50; height_image: 50
            onClicked:{rect_.y = 380; light.checked = true; mirror.checked = true; glass.checked = true; homescreenHandler.tapShortcut("steering"); root.modeLeft()}
        }

        ButtonControl {
            id: glass
            width: 50; height: 50; source: "images/glass.png"; source_: "images/line_devider.png"; width_image: 50; height_image: 50
            onClicked:{rect_.y = 460; steering.checked = true; mirror.checked = true; light.checked = true; root.modeLeft()}
        }
    }
}
