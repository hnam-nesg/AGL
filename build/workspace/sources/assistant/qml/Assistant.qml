import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import Qt5Compat.GraphicalEffects

Window {
    id: topbar
    width: 1280
    height: 800
    visible: panelOpen || wakeword.activeGraphic || settingsPreviewVisible
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowTransparentForInput

    property real audioLevel: wakeword.audioLevel
    property real timeValue: 0.0
    property real visualLevel: 0.0
    property bool panelDismissed: false
    property bool settingsPreviewVisible: false
    readonly property bool assistantWaitingForSpeech: wakeword.lastAssistantText.length > 0 &&
                                                      wakeword.visibleAssistantText.length === 0 &&
                                                      !wakeword.assistantThinking
    readonly property bool assistantDotsVisible: wakeword.assistantThinking ||
                                                 assistantWaitingForSpeech
    readonly property bool panelShouldOpen: wakeword.panelActive ||
                                            wakeword.assistantThinking
    readonly property bool panelOpen: panelShouldOpen && !panelDismissed

    onPanelShouldOpenChanged: {
        if (panelShouldOpen) {
            panelDismissed = false
            closePanelTimer.stop()
        }
    }

    Connections {
        target: wakeword

        function onConversationChanged() {
            closePanelTimer.stop()
        }

        function onPanelActiveChanged(active) {
            if (active) {
                topbar.panelDismissed = false
                closePanelTimer.stop()
            }
        }
    }

    Connections {
        target: assistantUiSettings

        function onSettingsChanged() {
            topbar.settingsPreviewVisible = true
            settingsPreviewTimer.restart()
        }
    }

    Timer {
        id: closePanelTimer
        interval: 2000
        repeat: false
        onTriggered: {
            topbar.panelDismissed = true
            wakeword.dismissPanel()
        }
    }

    Timer {
        id: settingsPreviewTimer
        interval: 2000
        repeat: false
        onTriggered: {
            topbar.settingsPreviewVisible = false
        }
    }

    // Background removed intentionally: this window must behave as a transparent overlay.
    // Background{width: 1280; height: 800}

    FontLoader{
        id: fontLoader
        source: "images/Roboto-BoldCondensed.ttf"
    }

    // Canvas {
    //     anchors.fill: parent
    //     antialiasing: true
    //     visible: false

    //     onPaint: {
    //         const ctx = getContext("2d");
    //         ctx.reset();

    //         const g = ctx.createLinearGradient(0, 0, width, 0);
    //         g.addColorStop(0.5, "#4e000000");
    //         g.addColorStop(1.0, "#2a000000");

    //         ctx.fillStyle = g;
    //         ctx.fillRect(0, 0, width, height);
    //     }
    // }

    Canvas {
        id: c
        anchors.fill: parent
        antialiasing: true
        z: 1
        visible: true

        // ===== STYLE =====
        property color strokeColor: "white"
        property real lineW: assistantUiSettings.lineWidth

        // ===== SHAPE =====
        property real pad: 50
        property real yBase: 3         // y ở ngoài biên
        property real gap: 40                      // khoảng hở giữa (để logo)
        property real centerDip: 50                // độ "tụt xuống" gần giữa
        property real curveTight: 0             // độ gắt cong gần giữa (0.3..0.8)

        // ===== FADE (mờ dần từ giữa ra biên) =====
        property real alphaCenter: assistantUiSettings.alphaCenter
        property real alphaEdge: assistantUiSettings.alphaEdge

        function rgbaStr(a) {
            // Canvas muốn rgba(...) dạng string
            const r = Math.round(strokeColor.r * 255)
            const g = Math.round(strokeColor.g * 255)
            const b = Math.round(strokeColor.b * 255)
            return "rgba(" + r + "," + g + "," + b + "," + a + ")"
        }

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()

            const dpr = Screen.devicePixelRatio || 1
            ctx.scale(dpr, dpr)

            const w = width
            const cx = w / 2

            const leftEndX   = cx - gap / 2
            const rightStartX = cx + gap / 2

            const yOuter  = yBase
            const yCenter = yBase + centerDip   // tụt xuống ở giữa

            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            // ---- helper: vẽ 1 nửa với gradient alpha ----
            function strokeHalf(isLeft) {
                ctx.beginPath()

                if (isLeft) {
                    // Gradient: ngoài biên nhạt -> gần giữa đậm
                    const gL = ctx.createLinearGradient(pad, 0, leftEndX, 0)
                    gL.addColorStop(0.0, rgbaStr(alphaEdge))
                    gL.addColorStop(1.0, rgbaStr(alphaCenter))
                    ctx.strokeStyle = gL

                    // Path: từ trái ngoài biên -> cong xuống và kết ở gần giữa
                    ctx.moveTo(pad, yOuter)

                    // cubic cho "đuôi" cong mềm rồi tụt xuống
                    const x1 = pad + (leftEndX - pad) * 1
                    const x2 = pad + (leftEndX - pad) * 0.92
                    ctx.bezierCurveTo(
                                x1, yOuter,                          // cp1 giữ ổn ngoài biên
                                x2, yOuter + centerDip * curveTight, // cp2 kéo xuống gần giữa
                                leftEndX, yCenter                    // end tụt xuống
                                )
                } else {
                    // Gradient: gần giữa đậm -> ra ngoài biên nhạt
                    const gR = ctx.createLinearGradient(rightStartX, 0, w - pad, 0)
                    gR.addColorStop(0.0, rgbaStr(alphaCenter))
                    gR.addColorStop(1.0, rgbaStr(alphaEdge))
                    ctx.strokeStyle = gR

                    ctx.moveTo(rightStartX, yCenter)

                    const x1 = rightStartX + (w - pad - rightStartX) * 0.08
                    const x2 = rightStartX + (w - pad - rightStartX) * 0
                    ctx.bezierCurveTo(
                                x1, yOuter + centerDip * curveTight, // cp1 kéo xuống gần giữa
                                x2, yOuter,                          // cp2 trở lại phẳng dần
                                w - pad, yOuter
                                )
                }

                ctx.lineWidth = lineW + 2.8
                const oldStroke = ctx.strokeStyle

                ctx.save()
                ctx.globalAlpha = 0.25
                ctx.stroke()
                ctx.restore()

                ctx.lineWidth = lineW
                ctx.strokeStyle = oldStroke
                ctx.stroke()
            }

            strokeHalf(true)
            strokeHalf(false)
        }

        function repaint() { requestPaint() }
        Component.onCompleted: repaint()
        onWidthChanged: repaint()
        onHeightChanged: repaint()
        onStrokeColorChanged: repaint()
        onLineWChanged: repaint()
        onPadChanged: repaint()
        onYBaseChanged: repaint()
        onGapChanged: repaint()
        onCenterDipChanged: repaint()
        onCurveTightChanged: repaint()
        onAlphaCenterChanged: repaint()
        onAlphaEdgeChanged: repaint()
    }

    // MultiEffect {
    //     id: effect_text
    //     anchors.fill: c
    //     source: c
    //     visible: false
    //     shadowEnabled: true
    //     shadowColor: "white"
    //     shadowOpacity: 1
    //     shadowBlur: 0.5
    //     shadowHorizontalOffset: 2
    //     shadowVerticalOffset: 2
    // }

    // Image{
    //     id: logo
    //     width: 55; height: 55
    //     anchors.centerIn: parent
    //     source: "images/mercedes.png"
    //     visible: false
    //     MultiEffect {
    //         anchors.fill: parent
    //         source: parent
    //         shadowEnabled: true
    //         shadowColor: "black"
    //         shadowOpacity: 1
    //         shadowBlur: 0.5
    //         shadowHorizontalOffset: 1
    //         shadowVerticalOffset: 1
    //     }
    // }

    // property var vi: Qt.locale("vi_VN")

    // Timer {
    //     id: timer
    //     interval: 1000; running: true; repeat: true
    //     onTriggered: {
    //         day_name.text = Qt.formatTime(new Date(), "HH:mm AP")
    //     }
    // }

    // Text {
    //     id: day_name
    //     x: 905; y: 10; width: 50; height: 30; font.pixelSize: 20
    //     horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; font.family: fontLoader.name; color: "white"
    //     visible: false
    // }
    // MultiEffect {
    //     visible: false
    //     anchors.fill: day_name; source: day_name;  shadowEnabled: true; shadowColor: "black"; shadowOpacity: 1; shadowBlur: 0.5; shadowHorizontalOffset: 5; shadowVerticalOffset: 5
    // }

    // Image{
    //     width: 25; height: 30; x: 1235; y: 10; source: "images/airbag.svg"; fillMode: Image.PreserveAspectCrop
    //     visible: false
    // }

    // Text{
    //     width: 50; height: 30; x: 1155; y: 10; font.family: fontLoader.name; horizontalAlignment: Text.AlignHLeft; verticalAlignment: Text.AlignVCenter
    //     text: "PASSENGER\nAIR BAG OFF"; font.pixelSize: 15; color: "#f9c200"
    //     visible: false
    // }

    // ButtonControl{
    //     x: 1105; y: 10; width: 30; height: 30; source: "images/mobile_phone.png"; width_image: 30; height_image: 30
    //     visible: false
    // }
    // ButtonControl{
    //     x: 1025; y: 10; width: 60; height: 30; source__: "images/account.png"; activeText: true; contentText: "Mercedes"; fontSize: 18
    //     visible: false
    // }

    Timer {
        interval: 16
        running: true
        repeat: true
        onTriggered: {
            topbar.timeValue += interval / 1000.0

            if (topbar.audioLevel > topbar.visualLevel)
                topbar.visualLevel = topbar.visualLevel * 0.65 + topbar.audioLevel * 0.35
            else
                topbar.visualLevel = topbar.visualLevel * 0.90 + topbar.audioLevel * 0.10
        }
    }

    ShaderEffect {
        id: underFx
        width: parent.width
        height: 50
        anchors.top: parent.top
        anchors.left: parent.left
        z: 0

        visible: topbar.panelOpen || wakeword.activeGraphic || topbar.settingsPreviewVisible

        property vector2d iResolution: Qt.vector2d(width, height)

        property real time: topbar.timeValue
        property real audioLevel: topbar.audioLevel

        property real pad: 50
        property real yBase: 3
        property real gap: 40
        property real centerDip: 50
        property real curveTight: 0

        property real fragThickness: assistantUiSettings.fragThickness
        property real fragGlow: assistantUiSettings.fragGlow
        property real brightness: assistantUiSettings.brightness
        property real normalOffset: assistantUiSettings.normalOffset
        property color shaderColor1: assistantUiSettings.shaderColor1
        property color shaderColor2: assistantUiSettings.shaderColor2
        property color shaderColor3: assistantUiSettings.shaderColor3

        fragmentShader: "qrc:/assistant.frag.qsb"
    }

    Rectangle {
        id: conversationPanel
        width: Math.min(parent.width * 0.7, 760)
        height: Math.min(parent.height - y - 28, Math.max(180, panelContent.implicitHeight + 36))
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: underFx.bottom
        anchors.topMargin: 16
        radius: 8
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#5A5A5A" }
            GradientStop { position: 1.0; color: "#2E2E2E" }
        }
        border.color: "#36ffffff"
        border.width: 1
        visible: topbar.panelOpen
        z: 2

        Column {
            id: panelContent
            anchors.fill: parent
            anchors.margins: 18
            spacing: 12

            Column {
                width: parent.width
                spacing: 5

                Text {
                    width: parent.width
                    text: "Người dùng"
                    color: "#F5E6B8"
                    font.family: fontLoader.name
                    font.pixelSize: 16
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    text: wakeword.lastUserText
                    color: "white"
                    font.family: fontLoader.name
                    font.pixelSize: 18
                    wrapMode: Text.Wrap
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: "#22ffffff"
            }

            Column {
                width: parent.width
                spacing: 5

                Text {
                    width: parent.width
                    text: "Trợ lý AI"
                    color: "#F5E6B8"
                    font.family: fontLoader.name
                    font.pixelSize: 16
                    elide: Text.ElideRight
                }

                Text {
                    id: assistantReplyText
                    width: parent.width
                    text: topbar.assistantDotsVisible ? "" : wakeword.visibleAssistantText
                    color: "#f3f7ff"
                    font.family: fontLoader.name
                    font.pixelSize: 18
                    lineHeight: 1.12
                    wrapMode: Text.Wrap
                    visible: !topbar.assistantDotsVisible &&
                             wakeword.visibleAssistantText.length > 0
                }
            }
        }

        Row {
            id: thinkingDots
            spacing: 7
            visible: topbar.assistantDotsVisible
            height: 26
            x: 30
            y: 130

            property real wavePhase: 0
            property real baseY: 10
            property real waveHeight: 7

            NumberAnimation on wavePhase {
                running: thinkingDots.visible
                from: 0
                to: Math.PI * 2
                duration: 1100
                loops: Animation.Infinite
                easing.type: Easing.Linear
            }

            Repeater {
                model: 3

                Item {
                    width: 7
                    height: 26

                    Rectangle {
                        width: 7
                        height: 7
                        radius: 4
                        color: "white"

                        property real s: Math.sin(thinkingDots.wavePhase - index * 0.7)

                        y: thinkingDots.baseY - Math.max(0, s) * thinkingDots.waveHeight

                        opacity: 0.35 + Math.max(0, s) * 0.65
                    }
                }
            }
        }

        Behavior on height {
            NumberAnimation {
                duration: 160
                easing.type: Easing.OutCubic
            }
        }
    }
}
