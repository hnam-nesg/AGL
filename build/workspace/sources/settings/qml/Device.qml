import QtQuick
import QtQuick.Effects

Item {
    id: root
    anchors.fill: parent

    property string selectedDeviceAddress: ""
    property string selectedDeviceName: ""

    property string pendingDeviceId: ""
    property string pendingAction: ""       // "pair", "connect", "disconnect", "remove"

    function bluezPathFromAddress(address) {
        if (!address || address.length === 0)
            return ""

        return "/org/bluez/hci0/dev_" + address.replace(/:/g, "_")
    }

    function deviceIdFromModel(m) {
        /*
         * Có model.id thì dùng model.id.
         * Log của bạn cho thấy model.id = dev_88_A4_79_65_91_34.
         * Backend AGL của bạn đang chấp nhận dạng này.
         */
        if (m.id && m.id.length > 0)
            return m.id

        if (m.address && m.address.length > 0)
            return bluezPathFromAddress(m.address)

        return ""
    }

    function requestBluetoothAction(action, devId, devName, devAddress) {
        if (!devId || devId.length === 0) {
            console.log("[Device.qml] requestBluetoothAction: empty devId, ignore")
            return
        }

        console.log("[Device.qml] requestBluetoothAction:",
                    "action=", action,
                    "devId=", devId,
                    "name=", devName,
                    "address=", devAddress)

        root.pendingAction = action
        root.pendingDeviceId = devId
        root.selectedDeviceName = devName ? devName : ""
        root.selectedDeviceAddress = devAddress ? devAddress : ""

        /*
         * Rất quan trọng:
         * Luôn stop discovery trước pair/connect/disconnect/remove.
         * Không phụ thuộc bluetooth.discovering vì property này có thể lệch BlueZ thật.
         */
        console.log("[Device.qml] force stop discovery before", action)
        bluetooth.stop_discovery()

        connectDelayTimer.restart()
    }

    Component.onCompleted: {
        console.log("[Device.qml] Bluetooth page loaded")
        console.log("[Device.qml] initial bluetooth.power =", bluetooth.power)
        bluetooth.refresh_devices()

        if (btn_bt.check !== bluetooth.power)
            btn_bt.check = bluetooth.power

        if (bluetooth.ready && bluetooth.power)
            delayedDiscoveryTimer.restart()

        /*
         * Không gọi bluetooth.start() ở đây.
         * BluetoothManager constructor đã gọi start() đúng một lần.
         * Không tự tắt Bluetooth khi mở trang, vì với device đã paired/known
         * BlueZ sẽ phải refresh/remove model ngay lúc UI đang dựng ListView.
         */
    }

    Timer {
        id: delayedDiscoveryTimer
        interval: 2500
        repeat: false

        onTriggered: {
            console.log("[Device.qml] delayed discovery start")

            if (!bluetooth.power) {
                console.log("[Device.qml] Bluetooth is not powered, skip discovery")
                return
            }

            if (!bluetooth.ready) {
                console.log("[Device.qml] Bluetooth backend is not ready, skip discovery")
                return
            }

            if (root.pendingAction.length > 0) {
                console.log("[Device.qml] pending action exists, skip discovery")
                return
            }

            bluetooth.discoverable = true
            bluetooth.start_discovery()
        }
    }

    Timer {
        id: connectDelayTimer
        interval: 1200
        repeat: false

        onTriggered: {
            if (root.pendingDeviceId.length === 0 || root.pendingAction.length === 0) {
                console.log("[Device.qml] connectDelayTimer: no pending action")
                return
            }

            console.log("[Device.qml] execute pending action:",
                        root.pendingAction,
                        root.pendingDeviceId)

            if (root.pendingAction === "pair") {
                bluetooth.pair(root.pendingDeviceId)
            } else if (root.pendingAction === "connect") {
                bluetooth.connect(root.pendingDeviceId)
            } else if (root.pendingAction === "disconnect") {
                bluetooth.disconnect(root.pendingDeviceId)
            } else if (root.pendingAction === "remove") {
                bluetooth.remove(root.pendingDeviceId)
            } else {
                console.log("[Device.qml] unknown pending action:", root.pendingAction)
            }

            /*
             * Không tự scan lại sau connect/remove để ưu tiên ổn định.
             * Nếu muốn tìm thiết bị mới, user bấm nút "Tìm thiết bị".
             */
            root.pendingAction = ""
            root.pendingDeviceId = ""
        }
    }

    Image {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 100
        width: 660
        height: 2
        source: "images/line_devider.png"
    }

    Connections {
        target: bluetooth

        function onRequestConfirmationEvent(pincode) {
            console.log("[Device.qml] request confirmation pincode:", pincode)

            /*
             * Tạm auto confirm như flow cũ.
             * Sau này muốn chuẩn hơn thì thay bằng popup xác nhận.
             */
            bluetooth.send_confirmation(pincode)
        }

        function onPowerChanged(state) {
            console.log("[Device.qml] onPowerChanged:", state)

            if (btn_bt.check !== state)
                btn_bt.check = state

            if (state) {
                delayedDiscoveryTimer.restart()
            } else {
                delayedDiscoveryTimer.stop()
                connectDelayTimer.stop()

                root.pendingAction = ""
                root.pendingDeviceId = ""

                bluetooth.stop_discovery()
                bluetooth.discoverable = false
            }
        }

        function onReadyChanged() {
            console.log("[Device.qml] onReadyChanged:", bluetooth.ready)

            if (bluetooth.ready) {
                bluetooth.refresh_devices()
                if (bluetooth.power)
                    delayedDiscoveryTimer.restart()
            }
        }

        function onDiscoverableChanged(state) {
            console.log("[Device.qml] onDiscoverableChanged:", state)
        }

        function onDiscoveringChanged() {
            console.log("[Device.qml] onDiscoveringChanged:", bluetooth.discovering)
        }

        function onErrorOccurred(message) {
            console.log("[Device.qml] Bluetooth error:", message)
        }
    }

    Row {
        width: parent.width
        height: 100
        spacing: 0

        ButtonControl {
            id: list_device
            width: parent.width / 2
            height: 100
            activeText: true
            contentText: "Danh sách thiết bị"
            fontSize: 25
        }

        ButtonControl {
            id: advance_settings
            width: parent.width / 2
            height: 100
            activeText: true
            contentText: "Cài đặt nâng cao"
            fontSize: 25
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 120
        font.family: fontLoader.name
        font.pixelSize: 20
        color: "white"

        text: bluetooth.power
              ? "Vào phần cài đặt Bluetooth trên thiết bị chọn \"raspberrypi5\"\nChọn một thiết bị khả dụng bên dưới để kết nối"
              : "Bluetooth đang tắt\nBật Bluetooth để tìm và kết nối thiết bị"

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Flickable {
        id: device
        x: 30
        y: 180
        width: 800
        height: 370
        clip: true
        flickableDirection: Flickable.VerticalFlick

        contentWidth: width
        contentHeight: contentColumn.height + 100
        interactive: true

        Column {
            id: contentColumn
            y: 20
            width: device.width
            spacing: 30
            visible: bluetooth.power 

            Text {
                id: text_connected
                font.family: fontLoader.name
                font.pixelSize: 20
                color: "white"
                visible: true
                text: "Đã ghép đôi / Đã biết"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            ListView {
                id: pairedList
                width: parent.width
                height: Math.max(1, count) * 90
                model: BluetoothPairedModel
                spacing: 10
                clip: true
                interactive: false

                delegate: Rectangle {
                    width: 680
                    height: 80
                    color: "#3f6f6f6f"
                    radius: 10

                    ButtonControl {
                        width: 680
                        height: 80
                        activeText: true

                        contentText: model.name && model.name.length > 0
                                     ? model.name
                                     : model.address

                        x_text: 70
                        fontSize: 25
                        source_v: "images/bluetooth.png"
                        align: true

                        onClicked: {
                            let devId = root.deviceIdFromModel(model)

                            console.log("[Device.qml] paired clicked:",
                                        model.name,
                                        "address=", model.address,
                                        "id=", model.id,
                                        "devId=", devId,
                                        "paired=", model.paired,
                                        "connected=", model.connected)

                            if (devId.length === 0) {
                                console.log("[Device.qml] Empty paired device id, ignore")
                                return
                            }

                            if (model.connected) {
                                root.requestBluetoothAction("disconnect",
                                                            devId,
                                                            model.name,
                                                            model.address)
                            } else {
                                root.requestBluetoothAction("connect",
                                                            devId,
                                                            model.name,
                                                            model.address)
                            }
                        }
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 120
                        anchors.verticalCenter: parent.verticalCenter
                        font.family: fontLoader.name
                        font.pixelSize: 16
                        color: "white"
                        text: model.connected ? "Ngắt" : "Kết nối"
                    }

                    ButtonControl {
                        id: forgetButton
                        anchors.right: parent.right
                        anchors.rightMargin: 20
                        anchors.verticalCenter: parent.verticalCenter

                        width: 80
                        height: 38
                        activeText: true
                        contentText: "Quên"
                        fontSize: 16

                        onClicked: {
                            let devId = root.deviceIdFromModel(model)

                            console.log("[Device.qml] forget/remove device clicked:",
                                        model.name,
                                        "address=", model.address,
                                        "id=", model.id,
                                        "devId=", devId,
                                        "paired=", model.paired,
                                        "connected=", model.connected)

                            if (devId.length === 0) {
                                console.log("[Device.qml] Empty paired device id, cannot remove")
                                return
                            }

                            root.requestBluetoothAction("remove",
                                                        devId,
                                                        model.name,
                                                        model.address)
                        }
                    }
                }
            }

            Row {
                width: parent.width
                height: 40
                spacing: 20

                Text {
                    width: 430
                    height: 40
                    font.family: fontLoader.name
                    font.pixelSize: 20
                    color: "white"

                    text: bluetooth.discovering
                          ? "Thiết bị khả dụng - đang quét..."
                          : "Thiết bị khả dụng"

                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }

                ButtonControl {
                    width: 230
                    height: 40
                    activeText: true
                    contentText: bluetooth.discovering ? "Dừng quét" : "Tìm thiết bị"
                    fontSize: 18

                    onClicked: {
                        if (!bluetooth.power) {
                            console.log("[Device.qml] Bluetooth off, cannot scan")
                            return
                        }

                        if (!bluetooth.ready) {
                            console.log("[Device.qml] Bluetooth backend not ready, cannot scan")
                            return
                        }

                        if (bluetooth.discovering) {
                            console.log("[Device.qml] user stop discovery")
                            bluetooth.stop_discovery()
                        } else {
                            console.log("[Device.qml] user start discovery")
                            bluetooth.discoverable = true
                            bluetooth.start_discovery()
                        }
                    }
                }
            }

            ListView {
                id: discoveryList
                width: parent.width
                height: Math.max(1, count) * 90
                model: BluetoothDiscoveryModel
                visible: bluetooth.power
                spacing: 10
                clip: true
                interactive: false

                delegate: Rectangle {
                    width: 680
                    height: 80
                    color: "#3f6f6f6f"
                    radius: 10

                    ButtonControl {
                        width: 680
                        height: 80
                        activeText: true

                        contentText: model.name && model.name.length > 0
                                     ? model.name
                                     : model.address

                        x_text: 70
                        fontSize: 25
                        source_v: "images/bluetooth.png"
                        align: true

                        onClicked: {
                            let devId = root.deviceIdFromModel(model)

                            console.log("[Device.qml] discovery clicked:",
                                        model.name,
                                        "address=", model.address,
                                        "id=", model.id,
                                        "devId=", devId,
                                        "paired=", model.paired,
                                        "connected=", model.connected)

                            if (devId.length === 0) {
                                console.log("[Device.qml] Empty discovery device id, ignore")
                                return
                            }

                            root.selectedDeviceAddress = model.address
                            root.selectedDeviceName = model.name

                            if (!model.paired) {
                                root.requestBluetoothAction("pair",
                                                            devId,
                                                            model.name,
                                                            model.address)
                            } else if (!model.connected) {
                                root.requestBluetoothAction("connect",
                                                            devId,
                                                            model.name,
                                                            model.address)
                            } else {
                                root.requestBluetoothAction("disconnect",
                                                            devId,
                                                            model.name,
                                                            model.address)
                            }
                        }
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 25
                        anchors.verticalCenter: parent.verticalCenter
                        font.family: fontLoader.name
                        font.pixelSize: 16
                        color: "white"

                        text: model.connected
                              ? "Ngắt"
                              : model.paired
                                ? "Kết nối"
                                : "Ghép đôi"
                    }
                }
            }
        }
    }

    SwitchButton {
        id: btn_bt
        y: 120
        x_: 630
        onCheckChanged: {
            console.log("[Device.qml] Switch bluetooth:", check)

            if (bluetooth.power === check)
                return

            if (!bluetooth.ready) {
                btn_bt.check = bluetooth.power
                return
            }

            bluetooth.power = check
        }
    }
}
