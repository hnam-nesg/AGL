import QtQuick
import QtQuick.Effects

Item {
    anchors.fill: parent
    Image{
        anchors.horizontalCenter: parent.horizontalCenter; y: 100; width: 660; height: 2; source: "images/line_devider.png"
    }

    Row{
        width: parent.width; height: 100; spacing: 0
        ButtonControl {
            id: list_device
            width: parent.width/2; height: 100; activeText: true; contentText: "Danh sách thiết bị"; fontSize: 25
        }
        ButtonControl {
            id: advance_settings
            width: parent.width/2; height: 100; activeText: true; contentText: "Cài đặt nâng cao"; fontSize: 25
        }
    }

    Text{
        anchors.horizontalCenter: parent.horizontalCenter; y: 120; font.family: fontLoader.name; font.pixelSize: 20; color: "white"
        text: "Vào phần cài đặt Bluetooth trên thiết bị chọn \"raspberrypi5\"\nChọn một thiết bị khả dụng bên dưới để kết nối"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
    }

    ListModel{
        id: bluetoothPairedModel
        ListElement{name: "Hoai Nam"}
        ListElement{name: "Hoai Nam"}
    }


    Flickable {
        id: device
        x: 30; y: 180; width: 800; height: 370; clip: true; flickableDirection: Flickable.VerticalFlick

        contentWidth: width
        contentHeight: column_device_connected.height + column_device_disconnected.height + 80
        interactive: true
        SwitchButton{id: btn_bt;x_: 600}
        Column{
            id: column_device_connected
            y: 20; width: device.width; spacing: 30; visible: btn_bt.check

            Text{
                id: text_connected
                font.family: fontLoader.name; font.pixelSize: 20; color: "white"; visible: true
                text: "Đang kết nối"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }

            ListView {
                id: connected_list
                width: parent.width
                height: 80*connected_list.count + 10 * (connected_list.count)
                model: bluetoothPairedModel
                visible: true//bluetooth.power
                spacing: 10
                delegate:Rectangle{
                        width: 680; height: 80; color: "#3f6f6f6f"; radius: 10
                        ButtonControl {
                            width: 680; height: 80; activeText: true; contentText: model.name; x_text: 70; fontSize: 25; source_v: "images/bluetooth.png"; align: true
                        }
                    }
                clip: true
            }
        }

        Column{
            id: column_device_disconnected
            y: column_device_connected.visible ? column_device_connected.height + 50 : 0; width: device.width; spacing: 30; visible: btn_bt.check

            Text{
                font.family: fontLoader.name; font.pixelSize: 20; color: "white"
                text: "Chưa kết nối"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
            }

            ListView {
                id: disconnected_list
                width: parent.width
                height: 80*disconnected_list.count + 10 * (disconnected_list.count)
                model: bluetoothPairedModel
                visible: true//bluetooth.power
                spacing: 10
                delegate:Rectangle{
                        width: 680; height: 80; color: "#3f6f6f6f"; radius: 10
                        ButtonControl {
                            width: 680; height: 80; activeText: true; contentText: model.name; x_text: 70; fontSize: 25; source_v: "images/bluetooth.png"; align: true
                        }
                    }
                clip: true
            }
        }
    }
}
