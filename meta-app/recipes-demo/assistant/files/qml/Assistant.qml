import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 720
    title: "AGL ASR QtQuick"
    color: "#0f172a"

    Component.onCompleted: asrManager.initializeEngine()

    header: Rectangle {
        height: 84
        color: "#111827"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16

            Label {
                text: "ASR Viewer"
                color: "white"
                font.pixelSize: 30
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                radius: 14
                color: asrManager.engineReady ? "#14532d" : "#7f1d1d"
                Layout.preferredHeight: 40
                Layout.preferredWidth: 170

                Label {
                    anchors.centerIn: parent
                    text: asrManager.engineReady ? "Engine Ready" : "Engine Not Ready"
                    color: "white"
                    font.pixelSize: 18
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        anchors.topMargin: header.height + 24
        spacing: 18

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 94
            radius: 20
            color: "#1e293b"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                ColumnLayout {
                    spacing: 6

                    Label {
                        text: "State"
                        color: "#94a3b8"
                        font.pixelSize: 18
                    }

                    Label {
                        text: asrManager.state
                        color: "white"
                        font.pixelSize: 28
                        font.bold: true
                    }
                }

                Rectangle {
                    width: 1
                    Layout.fillHeight: true
                    color: "#334155"
                }

                ColumnLayout {
                    spacing: 6

                    Label {
                        text: "Microphone"
                        color: "#94a3b8"
                        font.pixelSize: 18
                    }

                    Label {
                        text: asrManager.audioDeviceName.length > 0 ? asrManager.audioDeviceName : "Chưa mở microphone"
                        color: "white"
                        font.pixelSize: 22
                        elide: Label.ElideRight
                        Layout.preferredWidth: 460
                    }
                }

                Item { Layout.fillWidth: true }

                RowLayout {
                    spacing: 12

                    Button {
                        text: asrManager.listening ? "Listening..." : "Start"
                        enabled: !asrManager.listening
                        onClicked: asrManager.startListening()
                    }

                    Button {
                        text: "Stop"
                        enabled: asrManager.listening
                        onClicked: asrManager.stopListening()
                    }

                    Button {
                        text: "Clear"
                        onClicked: asrManager.clearTranscript()
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 180
            radius: 20
            color: "#1e293b"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                Label {
                    text: "Đang nhận diện"
                    color: "#93c5fd"
                    font.pixelSize: 22
                    font.bold: true
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true

                    Label {
                        width: parent.width
                        text: asrManager.partialText.length > 0 ? asrManager.partialText : "..."
                        color: "white"
                        wrapMode: Text.Wrap
                        font.pixelSize: 36
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120
            radius: 20
            color: "#1e293b"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 10

                Label {
                    text: "Kết quả cuối cùng"
                    color: "#86efac"
                    font.pixelSize: 22
                    font.bold: true
                }

                Label {
                    text: asrManager.finalText.length > 0 ? asrManager.finalText : "Chưa có câu nào được chốt"
                    color: "white"
                    wrapMode: Text.Wrap
                    font.pixelSize: 32
                    Layout.fillWidth: true
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 20
            color: "#1e293b"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                Label {
                    text: "Transcript"
                    color: "#fbbf24"
                    font.pixelSize: 22
                    font.bold: true
                }

                ListView {
                    id: transcriptView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 10
                    model: asrManager.transcriptLines

                    delegate: Rectangle {
                        width: transcriptView.width
                        radius: 14
                        color: "#334155"
                        height: textItem.implicitHeight + 24

                        Label {
                            id: textItem
                            anchors.fill: parent
                            anchors.margins: 12
                            text: modelData
                            color: "white"
                            wrapMode: Text.Wrap
                            font.pixelSize: 26
                        }
                    }

                    footer: Item { height: 8 }
                }
            }
        }
    }

    Popup {
        id: errorPopup
        x: (window.width - width) / 2
        y: 24
        width: 560
        height: contentItem.implicitHeight + 24
        modal: false
        focus: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            radius: 16
            color: "#7f1d1d"
            border.color: "#fca5a5"
        }

        contentItem: Label {
            id: errorLabel
            padding: 16
            color: "white"
            wrapMode: Text.Wrap
            font.pixelSize: 22
        }
    }

    Connections {
        target: asrManager

        function onErrorOccurred(message) {
            errorLabel.text = message
            errorPopup.open()
        }
    }
}
