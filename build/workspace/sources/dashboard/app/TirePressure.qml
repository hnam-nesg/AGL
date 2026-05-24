import QtQuick 2.6
import QtQuick.Controls 2.0

Image {
    id: root
    width: sourceSize.width
    height: sourceSize.height
    property bool mirror: false
    property alias title: title.text
    property alias pressure: pressure.text

    Label {
        id: title
        anchors.bottom: pressure.top
        font.pixelSize: 24
    }

    Label {
        id: pressure
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 55
        anchors.leftMargin: 140
        anchors.rightMargin: 140
        color: '#00ADDC'
        font.pixelSize: 20
    }

    states: [
        State {
            name: 'left'
            when: !mirror
            PropertyChanges {
                target: root
                source: 'qrc:/images/HMI_Dashboard_LeftTire.svg'
            }
            AnchorChanges {
                target: title
                anchors.right: pressure.right
            }
            AnchorChanges {
                target: pressure
                anchors.right: parent.right
            }
        },
        State {
            name: 'right'
            when: mirror
            PropertyChanges {
                target: root
                source: 'qrc:/images/HMI_Dashboard_RightTire.svg'
            }
            AnchorChanges {
                target: title
                anchors.left: pressure.left
            }
            AnchorChanges {
                target: pressure
                anchors.left: parent.left
            }
        }
    ]
}
