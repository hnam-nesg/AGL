import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent

    readonly property color accentColor: "#7FB0FF"
    readonly property color labelColor: "#F4F7FB"
    readonly property color valueColor: "#BFD5FF"
    readonly property color mutedColor: "#8FA0B5"
    readonly property color dividerColor: "#2B3A4D"

    FontLoader {
        id: fontLoader
        source: "images/Roboto-BoldCondensed.ttf"
    }

    ListModel {
        id: numericModel
        ListElement { label: "Độ dày nét"; key: "lineWidth"; minimum: 0.1; maximum: 12.0; step: 0.1; decimals: 1; suffix: "" }
        ListElement { label: "Độ mờ giữa"; key: "alphaCenter"; minimum: 0.0; maximum: 1.0; step: 0.05; decimals: 2; suffix: "" }
        ListElement { label: "Độ mờ rìa"; key: "alphaEdge"; minimum: 0.0; maximum: 1.0; step: 0.05; decimals: 2; suffix: "" }
        ListElement { label: "Độ dày shader"; key: "fragThickness"; minimum: 0.0; maximum: 20.0; step: 0.1; decimals: 1; suffix: "" }
        ListElement { label: "Độ phát sáng"; key: "fragGlow"; minimum: 0.0; maximum: 25.0; step: 0.1; decimals: 1; suffix: "" }
        ListElement { label: "Độ sáng"; key: "brightness"; minimum: 0.0; maximum: 3.0; step: 0.05; decimals: 2; suffix: "" }
        ListElement { label: "Độ lệch normal"; key: "normalOffset"; minimum: -100.0; maximum: 100.0; step: 0.5; decimals: 1; suffix: "" }
    }

    ListModel {
        id: colorModel
        ListElement { label: "Màu đầu"; key: "shaderColor1"; fallback: "#330099" }
        ListElement { label: "Màu giữa"; key: "shaderColor2"; fallback: "#004DFF" }
        ListElement { label: "Màu cuối"; key: "shaderColor3"; fallback: "#00FFFF" }
    }

    ButtonGroup {
        id: voiceGroup
    }

    function engineIndex() {
        return assistantSettings.engine === "piper" ? 1 : 0
    }

    function voiceIndex() {
        for (var i = 0; i < assistantSettings.voices.length; ++i) {
            if (assistantSettings.voices[i] === assistantSettings.voice)
                return i
        }
        return 0
    }

    function valueForKey(key) {
        switch (key) {
        case "lineWidth":
            return assistantSettings.lineWidth
        case "alphaCenter":
            return assistantSettings.alphaCenter
        case "alphaEdge":
            return assistantSettings.alphaEdge
        case "fragThickness":
            return assistantSettings.fragThickness
        case "fragGlow":
            return assistantSettings.fragGlow
        case "brightness":
            return assistantSettings.brightness
        case "normalOffset":
            return assistantSettings.normalOffset
        default:
            return 0.0
        }
    }

    function setValueForKey(key, value) {
        switch (key) {
        case "lineWidth":
            assistantSettings.lineWidth = value
            break
        case "alphaCenter":
            assistantSettings.alphaCenter = value
            break
        case "alphaEdge":
            assistantSettings.alphaEdge = value
            break
        case "fragThickness":
            assistantSettings.fragThickness = value
            break
        case "fragGlow":
            assistantSettings.fragGlow = value
            break
        case "brightness":
            assistantSettings.brightness = value
            break
        case "normalOffset":
            assistantSettings.normalOffset = value
            break
        }
    }

    function colorForKey(key) {
        switch (key) {
        case "shaderColor1":
            return assistantSettings.shaderColor1
        case "shaderColor2":
            return assistantSettings.shaderColor2
        case "shaderColor3":
            return assistantSettings.shaderColor3
        default:
            return "#000000"
        }
    }

    function setColorForKey(key, value) {
        switch (key) {
        case "shaderColor1":
            assistantSettings.shaderColor1 = value
            break
        case "shaderColor2":
            assistantSettings.shaderColor2 = value
            break
        case "shaderColor3":
            assistantSettings.shaderColor3 = value
            break
        }
    }

    function isValidHexColor(text) {
        return /^#[0-9a-fA-F]{6}$/.test(text)
    }

    // function openColorDialog(key) {
    //     effectColorDialog.openForKey(key)
    // }

    function resetColorForKey(key) {
        switch (key) {
        case "shaderColor1":
            assistantSettings.shaderColor1 = colorModel.get(0).fallback
            break
        case "shaderColor2":
            assistantSettings.shaderColor2 = colorModel.get(1).fallback
            break
        case "shaderColor3":
            assistantSettings.shaderColor3 = colorModel.get(2).fallback
            break
        }
    }

    Flickable {
        id: scroller
        anchors.fill: parent
        anchors.leftMargin: 40
        anchors.rightMargin: 34
        anchors.topMargin: 28
        anchors.bottomMargin: 28
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        contentWidth: width
        contentHeight: contentColumn.implicitHeight + 16
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

        Column {
            id: contentColumn
            width: scroller.width
            spacing: 22

            Row {
                width: parent.width
                height: 42
                spacing: 14

                Rectangle {
                    width: 4
                    height: 32
                    radius: 2
                    color: root.accentColor
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: "Giọng nói trợ lí"
                    color: "white"
                    font.family: fontLoader.name
                    font.pixelSize: 28
                    verticalAlignment: Text.AlignVCenter
                    height: parent.height
                }
            }

            // Rectangle {
            //     width: parent.width
            //     height: 1
            //     color: root.dividerColor
            // }

            GridLayout {
                width: parent.width
                columns: 2
                columnSpacing: 26
                rowSpacing: 20

                Text {
                    text: "Bộ đọc"
                    color: root.labelColor
                    font.family: fontLoader.name
                    font.pixelSize: 22
                    Layout.preferredWidth: 150
                    Layout.alignment: Qt.AlignVCenter
                }

                NeonComboBox {
                    id: engineBox
                    model: ["VieNeu", "Piper"]
                    currentIndex: root.engineIndex()
                    Layout.fillWidth: true
                    Layout.preferredHeight: 48
                    font.family: fontLoader.name
                    font.pixelSize: 18

                    uiFontFamily: fontLoader.name
                    glowColor: root.accentColor
                    //displayText: currentIndex === 1 ? "Piper" : "VieNeu"

                    property bool syncingFromSettings: false
                    
                     onCurrentIndexChanged: {
                        if (syncingFromSettings)
                            return

                        assistantSettings.engine = currentIndex === 1 ? "piper" : "vieneu"
                    }

                    delegate: Item {
                        id: engineDelegate

                        width: engineBox.width
                        height: 44

                        readonly property bool isCurrent: engineBox.currentIndex === index

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            radius: 10

                            color: engineDelegate.isCurrent
                                ? Qt.rgba(0.50, 0.69, 1.0, 0.32)
                                : engineMouse.containsMouse
                                    ? Qt.rgba(0.50, 0.69, 1.0, 0.18)
                                    : Qt.rgba(0.06, 0.10, 0.16, 0.96)

                            border.width: engineDelegate.isCurrent || engineMouse.containsMouse ? 1 : 0
                            border.color: Qt.rgba(0.70, 0.84, 1.0, 0.40)
                        }

                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 16
                            anchors.rightMargin: 12

                            text: modelData
                            color: "white"

                            font.family: fontLoader.name
                            font.pixelSize: 17

                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                            elide: Text.ElideRight
                        }

                        MouseArea {
                            id: engineMouse

                            anchors.fill: parent
                            hoverEnabled: true

                            onClicked: {
                                engineBox.currentIndex = index
                                engineBox.popup.close()
                            }

                        }
                    }


                    Connections {
                        target: assistantSettings
                        function onSettingsChanged() {
                            engineBox.syncingFromSettings = true
                            engineBox.currentIndex = root.engineIndex()
                            engineBox.syncingFromSettings = false
                        }
                    }
                }

                Text {
                    text: "Tốc độ"
                    color: root.labelColor
                    font.family: fontLoader.name
                    font.pixelSize: 22
                    Layout.preferredWidth: 150
                    Layout.alignment: Qt.AlignVCenter
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    spacing: 16

                    NeonSlider {
                        id: speedSlider
                        from: 0.75
                        to: 1.50
                        stepSize: 0.05
                        value: assistantSettings.speed
                        Layout.fillWidth: true
                        onValueChanged: assistantSettings.speed = value

                        Connections {
                            target: assistantSettings
                            function onSettingsChanged() {
                                speedSlider.value = assistantSettings.speed
                            }
                        }
                    }

                    Text {
                        text: assistantSettings.speed.toFixed(2) + "x"
                        color: root.valueColor
                        font.family: fontLoader.name
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignRight
                        Layout.preferredWidth: 72
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                Text {
                    text: "Âm lượng"
                    color: root.labelColor
                    font.family: fontLoader.name
                    font.pixelSize: 22
                    Layout.preferredWidth: 150
                    Layout.alignment: Qt.AlignVCenter
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    spacing: 16

                    NeonSlider {
                        id: volumeSlider
                        from: 0
                        to: 100
                        stepSize: 1
                        value: assistantSettings.volume
                        Layout.fillWidth: true
                        onValueChanged: assistantSettings.volume = Math.round(value)

                        Connections {
                            target: assistantSettings
                            function onSettingsChanged() {
                                volumeSlider.value = assistantSettings.volume
                            }
                        }
                    }

                    Text {
                        text: assistantSettings.volume + "%"
                        color: root.valueColor
                        font.family: fontLoader.name
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignRight
                        Layout.preferredWidth: 72
                        Layout.alignment: Qt.AlignVCenter
                    }
                }

                Text {
                    text: "Người nói"
                    color: assistantSettings.engine === "vieneu" ? root.labelColor : "#667386"
                    font.family: fontLoader.name
                    font.pixelSize: 22
                    Layout.preferredWidth: 150
                    Layout.alignment: Qt.AlignVCenter
                }

                NeonComboBox {
                    id: voiceBox

                    model: assistantSettings.voices
                    currentIndex: root.voiceIndex()

                    Layout.fillWidth: true
                    Layout.preferredHeight: 48

                    uiFontFamily: fontLoader.name
                    glowColor: root.accentColor

                    enabled: assistantSettings.engine === "vieneu"
                    opacity: assistantSettings.engine === "vieneu" ? 1.0 : 0.45

                    displayText: {
                        if (currentIndex >= 0 && currentIndex < assistantSettings.voices.length)
                            return assistantSettings.voiceDisplayName(assistantSettings.voices[currentIndex])
                        return "Không có voice"
                    }

                    delegate: Item {
                        id: voiceDelegate

                        width: voiceBox.width
                        height: 44

                        readonly property bool isCurrent: voiceBox.currentIndex === index

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            radius: 10

                            color: voiceDelegate.isCurrent
                                ? Qt.rgba(0.50, 0.69, 1.0, 0.32)
                                : voiceMouse.containsMouse
                                    ? Qt.rgba(0.50, 0.69, 1.0, 0.18)
                                    : Qt.rgba(0.06, 0.10, 0.16, 0.96)

                            border.width: voiceDelegate.isCurrent || voiceMouse.containsMouse ? 1 : 0
                            border.color: Qt.rgba(0.70, 0.84, 1.0, 0.40)
                        }

                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 16
                            anchors.rightMargin: 12

                            text: assistantSettings.voiceDisplayName(modelData)
                            color: "white"

                            font.family: fontLoader.name
                            font.pixelSize: 17

                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                            elide: Text.ElideRight
                        }

                        MouseArea {
                            id: voiceMouse

                            anchors.fill: parent
                            hoverEnabled: true

                            onClicked: {
                                voiceBox.currentIndex = index
                                assistantSettings.voice = assistantSettings.voices[index]
                                voiceBox.popup.close()
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 1
                color: root.dividerColor
            }

            Column {
                id: effectColorPanel
                width: parent.width
                spacing: 16

                property int activeColorIndex: 0

                function activeColorKey() {
                    if (activeColorIndex === 0)
                        return "shaderColor1"
                    if (activeColorIndex === 1)
                        return "shaderColor2"
                    return "shaderColor3"
                }

                function syncWheel() {
                    colorPicker.setFromHex(root.colorForKey(activeColorKey()))
                }

                function setActiveColor(index) {
                    activeColorIndex = index
                    syncWheel()
                }

                Component.onCompleted: {
                    Qt.callLater(function() {
                        effectColorPanel.syncWheel()
                    })
                }

                Row {
                    width: parent.width
                    height: 42
                    spacing: 14

                    Rectangle {
                        width: 4
                        height: 32
                        radius: 2
                        color: root.accentColor
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        width: parent.width
                        text: "Hiệu ứng"
                        color: "white"
                        font.family: fontLoader.name
                        font.pixelSize: 28
                    }
                }

                // Rectangle {
                //     width: parent.width
                //     height: 88
                //     radius: 16
                //     color: "#101826"
                //     border.width: 1
                //     border.color: "#42546B"
                //     clip: true

                //     gradient: Gradient {
                //         orientation: Gradient.Horizontal

                //         GradientStop {
                //             position: 0.0
                //             color: assistantSettings.shaderColor1
                //         }

                //         GradientStop {
                //             position: 0.5
                //             color: assistantSettings.shaderColor2
                //         }

                //         GradientStop {
                //             position: 1.0
                //             color: assistantSettings.shaderColor3
                //         }
                //     }

                //     Text {
                //         anchors.centerIn: parent
                //         text: "Preview màu hiệu ứng"
                //         color: "white"
                //         font.family: fontLoader.name
                //         font.pixelSize: 24
                //     }
                // }

                Rectangle {
                    width: parent.width
                    height: 400
                    radius: 20
                    color: "transparent"
                    ColumnLayout {
                        height: 150
                        width: 150
                        spacing: 20
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.horizontalCenterOffset: -200

                        ColorSelectButton {
                            text: "Màu đầu"
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            font.family: fontLoader.name
                            font.pixelSize: 16
                            onClicked: effectColorPanel.setActiveColor(0)

                            background: Rectangle {
                                radius: 12
                                color: assistantSettings.shaderColor1
                                border.width: effectColorPanel.activeColorIndex === 0 ? 3 : 1
                                border.color: effectColorPanel.activeColorIndex === 0 ? "white" : "#64748B"
                            }
                        }

                        ColorSelectButton {
                            text: "Màu giữa"
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            font.family: fontLoader.name
                            font.pixelSize: 16
                            onClicked: effectColorPanel.setActiveColor(1)

                            background: Rectangle {
                                radius: 12
                                color: assistantSettings.shaderColor2
                                border.width: effectColorPanel.activeColorIndex === 1 ? 3 : 1
                                border.color: effectColorPanel.activeColorIndex === 1 ? "white" : "#64748B"
                            }
                        }

                        ColorSelectButton {
                            text: "Màu cuối"
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            font.family: fontLoader.name
                            font.pixelSize: 16
                            onClicked: effectColorPanel.setActiveColor(2)

                            background: Rectangle {
                                radius: 12
                                color: assistantSettings.shaderColor3
                                border.width: effectColorPanel.activeColorIndex === 2 ? 3 : 1
                                border.color: effectColorPanel.activeColorIndex === 2 ? "white" : "#64748B"
                            }
                        }
                    }

                    Column {
                        anchors.fill: parent
                        anchors.margins: 18
                        spacing: 12

                        NeonColorWheelPicker {
                            id: colorPicker
                            width: 350
                            height: 350
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.right: parent.right

                            onEdited: function(hexColor) {
                                root.setColorForKey(effectColorPanel.activeColorKey(), hexColor)
                            }

                            onTouchStarted: {
                                scroller.interactive = false
                            }

                            onTouchEnded: {
                                scroller.interactive = true
                            }

                            Component.onCompleted: {
                                setFromHex(root.colorForKey(effectColorPanel.activeColorKey()))
                            }
                        }
                    }
                }
            }

            Column {
                width: parent.width
                spacing: 14

                Repeater {
                    model: numericModel

                    delegate: RowLayout {
                        width: parent.width
                        spacing: 16

                        Text {
                            text: label
                            color: root.labelColor
                            font.family: fontLoader.name
                            font.pixelSize: 22
                            Layout.preferredWidth: 150
                            Layout.alignment: Qt.AlignVCenter
                        }

                        NeonSlider {
                            id: valueSlider
                            from: minimum
                            to: maximum
                            stepSize: step
                            value: root.valueForKey(key)
                            Layout.fillWidth: true
                            Layout.preferredHeight: 50
                            onValueChanged: root.setValueForKey(key, value)

                            Connections {
                                target: assistantSettings
                                function onSettingsChanged() {
                                    valueSlider.value = root.valueForKey(key)
                                }
                            }
                        }

                        Text {
                            text: Number(root.valueForKey(key)).toFixed(decimals) + suffix
                            color: root.valueColor
                            font.family: fontLoader.name
                            font.pixelSize: 20
                            horizontalAlignment: Text.AlignRight
                            Layout.preferredWidth: 88
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }
                }
            }

            // Rectangle {
            //     width: parent.width
            //     height: 1
            //     color: root.dividerColor
            // }

            // RowLayout {
            //     width: parent.width
            //     spacing: 16

            //     Text {
            //         text: assistantSettings.settingsPath
            //         color: root.mutedColor
            //         font.family: fontLoader.name
            //         font.pixelSize: 15
            //         elide: Text.ElideMiddle
            //         Layout.fillWidth: true
            //         Layout.alignment: Qt.AlignVCenter
            //     }

            //     Button {
            //         text: "Tải lại"
            //         font.family: fontLoader.name
            //         font.pixelSize: 18
            //         Layout.preferredWidth: 110
            //         Layout.preferredHeight: 44
            //         onClicked: assistantSettings.reload()
            //     }
            // }
        }
    }
}
