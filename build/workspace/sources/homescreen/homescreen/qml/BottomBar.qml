import QtQuick
import QtQuick.Effects

Item {
    id: bottombar
    width: parent.width
    height: 80
    anchors.bottom: parent.bottom
    signal modeBottom()
    property alias drive_mode: driver_mode.text_color
    property alias home: home.source
    property alias menu: menu.source
    property alias camera: camera.source
    property alias fan: fan.source
    property int seat_driver: 0
    property int seat_passenger: 0
    property bool sync_temp: false
    property bool rear_frost: false
    property bool front_frost: false

    // Connections {
	// 	target: hvac
	// 	onFanSpeedChanged: fan.text_sub = fanSpeed
	// }

    function formatDisplayValue(value) {
        var v = Number(value)
        if (isNaN(v))
            return "0"
        return String(Math.floor(Math.abs(v)))
    }

    function clampSeatLevel(value) {
        var level = Number(value)
        if (isNaN(level))
            return 0
        //return Math.max(-3, Math.min(3, Math.round(level)))
        return level
    }

    function seatIconLevel(value) {
        var level = clampSeatLevel(value)
        return level === 0 ? 0 : -Math.abs(level)
    }

    function setActiveApp(appId) {
        var active = String(appId || "")

        driver_mode.text_color = active === "drivemode" ? "#0048e2" : "white"
        home.source = active === "homescreen" ? "images/home-on.png" : "images/home.png"
        camera.source = active === "camera" ? "images/camera-on.png" : "images/camera.png"
        fan.source = active === "hvac" ? "images/fan-on.png" : "images/fan.png"
        menu.source = active === "launcher" ? "images/menu-on.png" : "images/menu.png"
    }

    Connections {
        target: VehicleSignals

        function onConnected() {
            VehicleSignals.authorize()
        }

        function onAuthorized() {
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.Station.Row1.Driver.FanSpeed", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.Station.Row1.Driver.Temperature", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.Station.Row1.Passenger.Temperature", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.IsAirConditioningActive", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.IsRearDefrosterActive", true)
            VehicleSignals.subscribe("Vehicle.Cabin.HVAC.IsFrontDefrosterActive", true)
            VehicleSignals.subscribe("Vehicle.Cabin.Seat.Row1.DriverSide.HeatingCooling", true)
            VehicleSignals.subscribe("Vehicle.Cabin.Seat.Row1.PassengerSide.HeatingCooling", true)
        }


        function onSignalNotification(path, value, timestamp) {
            if (path === "Vehicle.Cabin.HVAC.Station.Row1.Driver.FanSpeed") {
                 fan.text_sub = bottombar.formatDisplayValue(value)
            }
            if (path === "Vehicle.Cabin.HVAC.Station.Row1.Driver.Temperature") {
                temp_driver.temp = Number(value)
            }
            if (path === "Vehicle.Cabin.HVAC.Station.Row1.Passenger.Temperature") {
                temp_passenger.temp = Number(value)
            }
            if (path === "Vehicle.Cabin.Seat.Row1.DriverSide.HeatingCooling") {
                bottombar.seat_driver = Number(value)
                seat_l.text_sub = bottombar.formatDisplayValue(value)
            }
            if (path === "Vehicle.Cabin.Seat.Row1.PassengerSide.HeatingCooling") {
                bottombar.seat_passenger = Number(value)
                seat_r.text_sub = bottombar.formatDisplayValue(value)
            }
            if (path === "Vehicle.Cabin.HVAC.IsAirConditioningActive") {
                var syncValue = (value === "true" || value === true)

                if (syncValue){
                    temp_passenger.temp = temp_driver.temp
                }
                
                bottombar.sync_temp = syncValue
                console.log("SyncTemp: ", bottombar.sync_temp)
            }
            if (path === "Vehicle.Cabin.HVAC.IsFrontDefrosterActive"){
                var activeValue = (value === "true" || value === true)
                
                bottombar.front_frost = activeValue
                console.log("FrontDefrost: ", bottombar.front_frost)
            }
            if (path === "Vehicle.Cabin.HVAC.IsRearDefrosterActive"){
                var activeValue = (value === "true" || value === true)
                
                bottombar.rear_frost = activeValue
                console.log("RearDefrost: ", bottombar.rear_frost)
            }
        }
    }

    Connections {
        target: launcher

        function onCurrentChanged(current) {
            bottombar.setActiveApp(current)
        }
    }

    Component.onCompleted: {
        VehicleSignals.connect()
        bottombar.setActiveApp(launcher.current)
    }

    Canvas {
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            const ctx = getContext("2d");
            ctx.reset();

            const g = ctx.createLinearGradient(0, 0, width, 0);
            g.addColorStop(0.5, "#4e000000");
            g.addColorStop(1.0, "#2a000000");


            ctx.fillStyle = g;
            ctx.fillRect(0, 0, width, height);
        }
        BorderColor{borderWidth: 1; radius: 0; scale: 1.01}
        // Rectangle {
        //     anchors.fill: parent
        //     scale: 1.01
        //     color: "transparent"
        //     layer.enabled: true
        //     layer.effect: MultiEffect {
        //         blurEnabled: true
        //         blur:0.6
        //     }
        //     BorderColor{borderWidth: 2}
        // }
    }

    Rectangle{
        x: 470; y: 10; width: 70; height: 70; color: "transparent"
        Canvas {
            id: blueGlowBg
            anchors.fill: parent
            antialiasing: true

            // "black"  -> mép tối đen (không thấy viền)
            // "transparent" -> mép fade ra trong suốt (nếu nền parent là transparent)
            property string edgeMode: "transparent"
            property real edgeFade: 1          // px: càng lớn càng fade mạnh ở mép
            property real glowStrength: 1.0

            onPaint: {
                const ctx = getContext("2d")
                const w = width, h = height
                if (ctx.reset) ctx.reset()
                ctx.clearRect(0, 0, w, h)

                // 1) Nền tối (linear đúng chuẩn 4 tham số)
                let base = ctx.createLinearGradient(0, 0, w, 0)
                base.addColorStop(0.00, "rgba(0,0,0,1)")
                base.addColorStop(0.55, "rgba(8,8,8,1)")
                base.addColorStop(1.00, "rgba(11,11,11,1)")
                ctx.fillStyle = base
                ctx.fillRect(0, 0, w, h)

                // 2) Glow xanh (radial)
                const gx = w * 0.50
                const gy = h * 0.5
                const R  = Math.max(w, h) * 0.75

                function drawGlow(alphaMul, radiusMul) {
                    const r = R * radiusMul
                    const g = ctx.createRadialGradient(gx, gy, 0, gx, gy, r)
                    g.addColorStop(0.00, `rgba(0, 85, 170, ${0.55*glowStrength*alphaMul})`)
                    g.addColorStop(0.22, `rgba(0, 55, 120, ${0.35*glowStrength*alphaMul})`)
                    g.addColorStop(0.55, `rgba(0, 28, 70,  ${0.14*glowStrength*alphaMul})`)
                    g.addColorStop(1.00, "rgba(0, 0, 0, 0)")
                    ctx.fillStyle = g
                    ctx.fillRect(0, 0, w, h)
                }

                // bloom giả blur: vẽ nhiều lớp
                ctx.globalCompositeOperation = "screen"
                drawGlow(0, 1.20)   // outer
                drawGlow(0.80, 0.95)   // mid
                drawGlow(1.00, 0.75)   // inner
                ctx.globalCompositeOperation = "source-over"

                // (bỏ sweep cam/orange; nếu muốn sweep thì dùng xanh/đen, không dùng cam)
                // -----

                // 3) EDGE MASK: ÉP MẺP VỀ ĐEN hoặc TRANSPARENT để không thấy viền
                if (edgeMode === "black") {
                    // Vignette đen: mép tối dần
                    const m = Math.max(w, h) * 0.3
                    const mask = ctx.createRadialGradient(w*0.5, h*0.5, m - edgeFade, w*0.5, h*0.5, m)
                    mask.addColorStop(0.00, "rgba(0,0,0,0)")
                    mask.addColorStop(0.80, "rgba(0,0,0,0.25)")
                    mask.addColorStop(1.00, "rgba(0,0,0,0.85)")
                    ctx.fillStyle = mask
                    ctx.fillRect(0, 0, w, h)
                } else if (edgeMode === "transparent") {
                    // Alpha mask: fade ra trong suốt ở mép (cần parent/background phía sau)
                    ctx.globalCompositeOperation = "destination-in"
                    const m = Math.max(w, h) * 0.45
                    const a = ctx.createRadialGradient(w*0.5, h*0.5, m - edgeFade, w*0.5, h*0.5, m)
                    a.addColorStop(0.00, "rgba(0,0,0,1)")
                    a.addColorStop(0.85, "rgba(0,0,0,1)")
                    a.addColorStop(1.00, "rgba(0,0,0,0)")
                    ctx.fillStyle = a
                    ctx.fillRect(0, 0, w, h)
                    ctx.globalCompositeOperation = "source-over"
                }
            }

            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
            Component.onCompleted: requestPaint()
        }

    }

    Image{
        x: 460; y: 78.5; width: 100; height: 3; source: "images/line_devider.png"
    }

    Rectangle{
        x: 740; y: 10; width: 70; height: 70; color: "transparent"
        Canvas {
            id: blueGlowBg1
            anchors.fill: parent
            antialiasing: true

            // "black"  -> mép tối đen (không thấy viền)
            // "transparent" -> mép fade ra trong suốt (nếu nền parent là transparent)
            property string edgeMode: "transparent"
            property real edgeFade: 1          // px: càng lớn càng fade mạnh ở mép
            property real glowStrength: 1.0

            onPaint: {
                const ctx = getContext("2d")
                const w = width, h = height
                if (ctx.reset) ctx.reset()
                ctx.clearRect(0, 0, w, h)

                // 1) Nền tối (linear đúng chuẩn 4 tham số)
                let base = ctx.createLinearGradient(0, 0, w, 0)
                base.addColorStop(0.00, "rgba(0,0,0,1)")
                base.addColorStop(0.55, "rgba(8,8,8,1)")
                base.addColorStop(1.00, "rgba(11,11,11,1)")
                ctx.fillStyle = base
                ctx.fillRect(0, 0, w, h)

                // 2) Glow xanh (radial)
                const gx = w * 0.50
                const gy = h * 0.5
                const R  = Math.max(w, h) * 0.75

                function drawGlow(alphaMul, radiusMul) {
                    const r = R * radiusMul
                    const g = ctx.createRadialGradient(gx, gy, 0, gx, gy, r)
                    g.addColorStop(0.00, `rgba(0, 85, 170, ${0.55*glowStrength*alphaMul})`)
                    g.addColorStop(0.22, `rgba(0, 55, 120, ${0.35*glowStrength*alphaMul})`)
                    g.addColorStop(0.55, `rgba(0, 28, 70,  ${0.14*glowStrength*alphaMul})`)
                    g.addColorStop(1.00, "rgba(0, 0, 0, 0)")
                    ctx.fillStyle = g
                    ctx.fillRect(0, 0, w, h)
                }

                // bloom giả blur: vẽ nhiều lớp
                ctx.globalCompositeOperation = "screen"
                drawGlow(0, 1.20)   // outer
                drawGlow(0.80, 0.95)   // mid
                drawGlow(1.00, 0.75)   // inner
                ctx.globalCompositeOperation = "source-over"

                // (bỏ sweep cam/orange; nếu muốn sweep thì dùng xanh/đen, không dùng cam)
                // -----

                // 3) EDGE MASK: ÉP MẺP VỀ ĐEN hoặc TRANSPARENT để không thấy viền
                if (edgeMode === "black") {
                    // Vignette đen: mép tối dần
                    const m = Math.max(w, h) * 0.3
                    const mask = ctx.createRadialGradient(w*0.5, h*0.5, m - edgeFade, w*0.5, h*0.5, m)
                    mask.addColorStop(0.00, "rgba(0,0,0,0)")
                    mask.addColorStop(0.80, "rgba(0,0,0,0.25)")
                    mask.addColorStop(1.00, "rgba(0,0,0,0.85)")
                    ctx.fillStyle = mask
                    ctx.fillRect(0, 0, w, h)
                } else if (edgeMode === "transparent") {
                    // Alpha mask: fade ra trong suốt ở mép (cần parent/background phía sau)
                    ctx.globalCompositeOperation = "destination-in"
                    const m = Math.max(w, h) * 0.45
                    const a = ctx.createRadialGradient(w*0.5, h*0.5, m - edgeFade, w*0.5, h*0.5, m)
                    a.addColorStop(0.00, "rgba(0,0,0,1)")
                    a.addColorStop(0.85, "rgba(0,0,0,1)")
                    a.addColorStop(1.00, "rgba(0,0,0,0)")
                    ctx.fillStyle = a
                    ctx.fillRect(0, 0, w, h)
                    ctx.globalCompositeOperation = "source-over"
                }
            }

            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
            Component.onCompleted: requestPaint()
        }
    }

    Image{
        x: 730; y: 78.5; width: 100; height: 3; source: "images/line_devider.png"
    }

    Canvas {
        id: c
        anchors.fill: parent
        antialiasing: true
        z: 1

        // ===== STYLE =====
        property color strokeColor: "white"
        property real lineW: 1

        // ===== SHAPE =====
        property real pad: 150
        property real yBase: 15         // y ở ngoài biên
        property real gap: 2.2                    // khoảng hở giữa (để logo)
        property real centerDip: 10                // độ "tụt xuống" gần giữa
        property real curveTight: 0             // độ gắt cong gần giữa (0.3..0.8)

        // ===== FADE (mờ dần từ giữa ra biên) =====
        property real alphaCenter: 1.0
        property real alphaEdge: 0

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
            const yCenter = yBase - centerDip   // tụt xuống ở giữa

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

                // (Tuỳ chọn) vẽ glow mỏng phía ngoài (giống line có halo)
                ctx.lineWidth = lineW + 2.8
                const oldStroke = ctx.strokeStyle
                // giảm alpha glow
                // Với gradient, ta vẽ thêm 1 lần với globalAlpha
                ctx.save()
                ctx.globalAlpha = 0.25
                ctx.stroke()
                ctx.restore()

                // vẽ line chính
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

    MultiEffect {
        id: effect_text
        anchors.fill: c
        source: c
        shadowEnabled: true
        shadowColor: "white"
        shadowOpacity: 1
        shadowBlur: 0.5
        shadowHorizontalOffset: 1
        shadowVerticalOffset: 3
    }

    ButtonControl {
        id: home
        x: 20; y: 15; width: 60; height: 60; source: "images/home.png"; width_image: 50; height_image: 50
        onClicked:{driver_mode.text_color = "white"; fan.source = "images/fan.png"; camera.source = "images/camera.png"; home.source = "images/home-on.png"; menu.source = "images/menu.png"; bottombar.modeBottom(); homescreenHandler.tapShortcut("homescreen")}
    }

    ButtonControl {
        id: driver_mode
        x: 115; y: 20; width: 70; height: 50; activeText: true; contentText: "DRIVE<br/>MODE"; fontSize: 18; width_image: 50; height_image: 50; source_v: "images/line_devider.png"; rotation: 90;
        onClicked:{driver_mode.text_color = "#0048e2"; camera.source = "images/camera.png"; home.source = "images/home.png"; menu.source = "images/menu.png"; bottombar.modeBottom(); homescreenHandler.tapShortcut("drivemode")}
    }

    ButtonControl {
        id: camera
        x: 220; y: 15; width: 60; height: 60; source: "images/camera.png"; width_image: 50; height_image: 50
        onClicked:{driver_mode.text_color = "white"; fan.source = "images/fan.png"; camera.source = "images/camera-on.png"; home.source = "images/home.png"; menu.source = "images/menu.png"; bottombar.modeBottom(); homescreenHandler.tapShortcut("camera")}
    }

    ButtonControl {
        id: seat_l
        x: 320; y: 15; width: 60; height: 60; source_v: "images/line_devider.png"; rotation: 90; source: "images/seat_l_"+ bottombar.seat_driver +".png"
        text_direct: -30; text_sub: "0"; width_image: 40; height_image: 40
        onClicked:{driver_mode.text_color = "white"; camera.source = "images/camera.png"; home.source = "images/home.png"; menu.source = "images/menu.png"; fan.source = "images/fan-on.png" ;bottombar.modeBottom(); homescreenHandler.tapShortcut("hvac")}
    }

    ButtonControl {
        id: down_l
        x: 420; y: 20; width: 50; height: 50; source: "images/ico_arrow_d_p.png"; width_image: 30; height_image: 30
        onClicked:{
            if(temp_driver.temp <= 30 && temp_driver.temp > 8) temp_driver.temp -= 0.5
        }
    }


    ButtonControl {
        id: temp_driver
        property real temp: 19
        x: 480; y: 20; width: 50; height: 50; activeText: true; contentText: temp + "°C"; fontSize: 24; enabled: false; width_image: 50; height_image: 50
        onClicked:{driver_mode.text_color = "white"; fan.source = "images/fan.png"; camera.source = "images/camera.png"; home.source = "images/home.png"; menu.source = "images/menu.png"; bottombar.modeBottom(); homescreenHandler.tapShortcut("hvac")}
    }

    ButtonControl {
        id: up_l
        x: 545; y: 18; width: 50; height: 50; source: "images/ico_arrow_u_p.png"; width_image: 30; height_image: 30
        onClicked:{
            if(temp_driver.temp >=8 && temp_driver.temp < 30) temp_driver.temp += 0.5
        }
    }

    ButtonControl {
        id: fan
        x: 615; y: 20; width: 50; height: 50; source: "images/fan.png"; text_sub: "0"; text_direct: 35; text_sub_size: 25; width_image: 30; height_image: 30
        onClicked:{driver_mode.text_color = "white"; source = "images/fan-on.png";camera.source = "images/camera.png"; home.source = "images/home.png";menu.source = "images/menu.png"; bottombar.modeBottom(); homescreenHandler.tapShortcut("hvac")}
    }

    ButtonControl {
        id: down_r
        x: 690; y: 20; width: 50; height: 50; source: "images/ico_arrow_d_p.png"; width_image: 30; height_image: 30
        onClicked:{
            if(temp_passenger.temp <= 30 && temp_passenger.temp > 8 && !bottombar.sync_temp) temp_passenger.temp -= 0.5
            else{temp_driver.temp -= 0.5}
        }
    }

    ButtonControl {
        id: temp_passenger
        property real temp: 14
        x: 750; y: 20; width: 50; height: 50; activeText: true; contentText: bottombar.sync_temp ? (temp_driver.temp + "°C") : (temp + "°C"); fontSize: 24; enabled: false; width_image: 50; height_image: 50
        onClicked:{driver_mode.text_color = "white"; fan.source = "images/fan.png"; camera.source = "images/camera.png"; home.source = "images/home.png";menu.source = "images/menu.png"; bottombar.modeBottom(); homescreenHandler.tapShortcut("hvac")}
    }

    ButtonControl {
        id: up_r
        x: 815; y: 18; width: 50; height: 50; source: "images/ico_arrow_u_p.png"; width_image: 30; height_image: 30
        onClicked:{
            if(temp_passenger.temp >=8 && temp_passenger.temp < 30 && !bottombar.sync_temp) temp_passenger.temp += 0.5
            else temp_driver.temp += 0.5
        }
    }

    ButtonControl {
        id: seat_r
        x: 910; y: 15; width: 60; height: 60; source: "images/seat_r_"+ bottombar.seat_passenger +".png"; text_sub: "0"; text_direct: 30; width_image: 40; height_image: 40
        onClicked:{driver_mode.text_color = "white"; camera.source = "images/camera.png"; home.source = "images/home.png";menu.source = "images/menu.png"; fan.source = "images/fan-on.png"; bottombar.modeBottom(); homescreenHandler.tapShortcut("hvac")}
    }

    ButtonControl {
        id: heat_front
        x: 1010; y: 15; width: 60; height: 60; source: checked || bottombar.front_frost ? "images/front_on.png" : "images/front.png"; source_v: "images/line_devider.png"; rotation: 90; width_image: 50; height_image: 50
        onClicked:{hvac.frontDefrost = checked}
    }

    ButtonControl {
        id: heat_rear
        x: 1110; y: 15; width: 60; height: 60; source: checked || bottombar.rear_frost ? "images/rear_on.png" : "images/rear.png"; width_image: 50; height_image: 50
        onClicked:{hvac.rearDefrost = checked}
    }

    ButtonControl {
        id: menu
        x: 1210; y: 15; width: 60; height: 60; source: "images/menu.png"; source_v: "images/line_devider.png"; rotation: 90; width_image: 40; height_image: 40
        onClicked:{driver_mode.text_color = "white"; fan.source = "images/fan.png"; camera.source = "images/camera.png"; home.source = "images/home.png";menu.source = "images/menu-on.png"; bottombar.modeBottom(); homescreenHandler.tapShortcut("launcher")}
    }

}
