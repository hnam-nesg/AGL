import QtQuick
import QtQuick.Controls.Basic as Control
import Qt5Compat.GraphicalEffects

Rectangle {
    id: keyboard
    width: 780
    height: 252
    radius: 22
    color: "#eef0f3"
    border.width: 1
    border.color: "#d8dadd"

    property bool symbols: false
    property var letterRows: [
        ["q", "w", "e", "r", "t", "y", "u", "i", "o", "p"],
        ["a", "s", "d", "f", "g", "h", "j", "k", "l"],
        ["mode", "z", "x", "c", "v", "b", "n", "m", "backspace"],
        ["space", "slash", "hide", "search"]
    ]
    property var symbolRows: [
        ["1", "2", "3", "4", "5", "6", "7", "8", "9", "0"],
        ["@", "#", "$", "_", "&", "-", "+", "(", ")"],
        ["mode", "/", "*", "\"", "'", ":", ";", "!", "backspace"],
        ["space", "comma", "dot", "hide", "search"]
    ]
    property var activeRows: symbols ? symbolRows : letterRows

    signal insertText(string text)
    signal backspacePressed()
    signal searchPressed()
    signal hidePressed()

    function labelFor(value) {
        if (value === "mode")
            return keyboard.symbols ? "ABC" : "123"
        if (value === "backspace")
            return "⌫"
        if (value === "space")
            return ""
        if (value === "hide")
            return "⌄"
        if (value === "search")
            return "Tìm"
        if (value === "slash")
            return "/"
        if (value === "comma")
            return ","
        if (value === "dot")
            return "."
        return value
    }

    function keyWidth(value) {
        if (value === "space")
            return 390
        if (value === "search")
            return 104
        if (value === "hide" || value === "backspace")
            return 70
        if (value === "mode")
            return 68
        return 56
    }

    function handleKey(value) {
        if (value === "mode") {
            keyboard.symbols = !keyboard.symbols
            return
        }
        if (value === "backspace") {
            keyboard.backspacePressed()
            return
        }
        if (value === "space") {
            keyboard.insertText(" ")
            return
        }
        if (value === "hide") {
            keyboard.hidePressed()
            return
        }
        if (value === "search") {
            keyboard.searchPressed()
            return
        }
        if (value === "slash") {
            keyboard.insertText("/")
            return
        }
        if (value === "comma") {
            keyboard.insertText(",")
            return
        }
        if (value === "dot") {
            keyboard.insertText(".")
            return
        }
        keyboard.insertText(value)
    }

    layer.enabled: true
    layer.effect: DropShadow {
        horizontalOffset: 0
        verticalOffset: -3
        radius: 20
        samples: 36
        color: "#30000000"
    }

    Column {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        anchors.topMargin: 12
        anchors.bottomMargin: 14
        spacing: 9

        Rectangle {
            width: 46
            height: 4
            radius: 2
            color: "#c6c9ce"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Repeater {
            model: keyboard.activeRows

            Row {
                id: keyRow
                height: 45
                spacing: 7
                anchors.horizontalCenter: parent.horizontalCenter

                Repeater {
                    model: modelData

                    Control.Button {
                        id: keyButton
                        width: keyboard.keyWidth(modelData)
                        height: keyRow.height
                        text: keyboard.labelFor(modelData)
                        focusPolicy: Qt.NoFocus

                        background: Rectangle {
                            radius: modelData === "space" ? 21 : 12
                            color: modelData === "search"
                                   ? (keyButton.pressed ? "#1558d6" : "#1a73e8")
                                   : (keyButton.pressed ? "#e0e3e7" : "#ffffff")
                            border.width: 0

                            layer.enabled: modelData !== "space"
                            layer.effect: DropShadow {
                                horizontalOffset: 0
                                verticalOffset: 1
                                radius: 3
                                samples: 8
                                color: "#18000000"
                            }
                        }

                        contentItem: Text {
                            text: keyButton.text
                            color: modelData === "search" ? "#ffffff" : "#202124"
                            font.pixelSize: modelData === "search" ? 15 : (modelData === "backspace" || modelData === "hide" ? 20 : 18)
                            font.bold: modelData === "search"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }

                        onClicked: keyboard.handleKey(modelData)
                    }
                }
            }
        }
    }
}
