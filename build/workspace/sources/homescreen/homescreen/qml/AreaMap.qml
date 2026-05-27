import QtQuick
import QtPositioning
import QtQuick.Effects
import Qt5Compat.GraphicalEffects

Rectangle {
    id: areaMap
    width: 430
    height: 640
    radius: 12
    color: "#07111d"
    clip: true

    property int headerHeight: Math.round(height * 0.20)
    property date now: new Date()
    property var routePath: []
    property string routePathKey: ""
    property int routePointCount: 0
    property int routeSessionSeed: 0
    property int routeResetSeed: 0
    property bool routeVisible: navigationActive && !routeArrived && routePointCount > 1
    property bool localRouteArrived: false
    property bool deferredContentActive: true
    property bool uiReady: mapLoader.status === Loader.Error
                           || (mapLoader.status === Loader.Ready
                               && mapLoader.item
                               && mapLoader.item.loadComplete)
    property var defaultCoord: QtPositioning.coordinate(10.80690, 106.70290)
    property bool navigationActive: homescreenHandler.navigationActive
    property bool routeArrived: localRouteArrived || navigationActive
                                && /^0(?:\.0+)?\s*(m|km)?$/i.test(cleanText(homescreenHandler.navigationDistanceText))
    property var currentCoord: validCoordinate(homescreenHandler.navigationCurrentLatitude,
                                               homescreenHandler.navigationCurrentLongitude)
                               ? QtPositioning.coordinate(homescreenHandler.navigationCurrentLatitude,
                                                          homescreenHandler.navigationCurrentLongitude)
                               : defaultCoord
    property var startCoord: validCoordinate(homescreenHandler.navigationStartLatitude,
                                             homescreenHandler.navigationStartLongitude)
                             ? QtPositioning.coordinate(homescreenHandler.navigationStartLatitude,
                                                        homescreenHandler.navigationStartLongitude)
                             : currentCoord
    property var destinationCoord: validCoordinate(homescreenHandler.navigationDestinationLatitude,
                                                   homescreenHandler.navigationDestinationLongitude)
                                   ? QtPositioning.coordinate(homescreenHandler.navigationDestinationLatitude,
                                                              homescreenHandler.navigationDestinationLongitude)
                                   : currentCoord

    FontLoader {
        id: mapFont
        source: "images/Roboto-BoldCondensed.ttf"
    }

    Timer {
        interval: 60000
        repeat: true
        running: true
        triggeredOnStart: false
        onTriggered: areaMap.now = new Date()
    }

    function miniMapItem() {
        return mapLoader.item
    }

    function validCoordinate(latitude, longitude) {
        return isFinite(latitude) && isFinite(longitude)
                && latitude >= -90 && latitude <= 90
                && longitude >= -180 && longitude <= 180
                && !(Math.abs(latitude) < 0.000001 && Math.abs(longitude) < 0.000001)
    }

    function coordValid(coord) {
        return coord
                && validCoordinate(Number(coord.latitude), Number(coord.longitude))
    }

    function cleanText(value) {
        if (value === undefined || value === null)
            return ""
        return String(value).trim()
    }

    function toCoordinate(point) {
        if (!point)
            return defaultCoord
        return QtPositioning.coordinate(Number(point.latitude), Number(point.longitude))
    }

    function normalizedBearing(value) {
        var bearing = Number(value) % 360
        if (!isFinite(bearing))
            return 0
        return bearing < 0 ? bearing + 360 : bearing
    }

    function boundedZoom(value) {
        var map = miniMapItem()
        var minZoom = map && isFinite(map.minimumZoomLevel) ? map.minimumZoomLevel : 2
        var maxZoom = map && isFinite(map.maximumZoomLevel) ? map.maximumZoomLevel : 20
        return Math.max(minZoom, Math.min(maxZoom, value))
    }

    function boundedTilt(value) {
        var map = miniMapItem()
        var minTilt = map && isFinite(map.minimumTilt) ? map.minimumTilt : 0
        var maxTilt = map && isFinite(map.maximumTilt) ? map.maximumTilt : 60
        return Math.max(minTilt, Math.min(maxTilt, value))
    }

    function moveAhead(coord, bearingDeg, distM) {
        if (!coordValid(coord))
            return coord

        var r = 6371000.0
        var br = bearingDeg * Math.PI / 180
        var lat1 = coord.latitude * Math.PI / 180
        var lon1 = coord.longitude * Math.PI / 180
        var d = distM / r
        var lat2 = Math.asin(Math.sin(lat1) * Math.cos(d)
                             + Math.cos(lat1) * Math.sin(d) * Math.cos(br))
        var lon2 = lon1 + Math.atan2(Math.sin(br) * Math.sin(d) * Math.cos(lat1),
                                     Math.cos(d) - Math.sin(lat1) * Math.sin(lat2))
        return QtPositioning.coordinate(lat2 * 180 / Math.PI, lon2 * 180 / Math.PI)
    }

    function guidanceCenter() {
        return moveAhead(currentCoord,
                         normalizedBearing(homescreenHandler.navigationHeading + 180),
                         42)
    }

    function isHoChiMinh(coord) {
        return coordValid(coord)
                && coord.latitude > 10.35 && coord.latitude < 11.25
                && coord.longitude > 106.25 && coord.longitude < 107.15
    }

    function currentPlaceTitle() {
        if (isHoChiMinh(currentCoord))
            return qsTr("TP. Hồ Chí Minh")
        return qsTr("Vị trí hiện tại")
    }

    function currentPlaceSubtitle() {
        if (isHoChiMinh(currentCoord))
            return qsTr("Bình Thạnh")
        return shortCoord(currentCoord)
    }

    function shortCoord(coord) {
        if (!coordValid(coord))
            return qsTr("Chưa có tọa độ")
        return coord.latitude.toFixed(5) + ", " + coord.longitude.toFixed(5)
    }

    function two(value) {
        return value < 10 ? "0" + value : "" + value
    }

    function dateText() {
        var days = [
            qsTr("Chủ nhật"),
            qsTr("Thứ hai"),
            qsTr("Thứ ba"),
            qsTr("Thứ tư"),
            qsTr("Thứ năm"),
            qsTr("Thứ sáu"),
            qsTr("Thứ bảy")
        ]
        return days[now.getDay()] + ", "
                + two(now.getDate()) + "/"
                + two(now.getMonth() + 1) + "/"
                + now.getFullYear()
    }

    function weatherTemperatureText() {
        if (typeof weather === "undefined")
            return "--"

        var value = cleanText(weather.temperature)
        if (value.length === 0)
            return "--"
        return /[°CFcf]/.test(value) ? value : (value - 32 )/1.8 +  "°C"
    }

    function weatherConditionText() {
        if (typeof weather === "undefined")
            return qsTr("Đang cập nhật")

        var value = cleanText(weather.condition)
        return value.length > 0 ? value : qsTr("Đang cập nhật")
    }

    function weatherKind() {
        var value = weatherConditionText().toLowerCase()
        if (/(mưa|rain|shower|drizzle|storm|thunder|dông|giông)/i.test(value))
            return "rain"
        if (/(nắng|sun|clear|fair)/i.test(value))
            return "sun"
        if (/(mây|cloud|overcast|sương|fog|mist)/i.test(value))
            return "cloud"
        return "sun"
    }

    function destinationTitleText() {
        var name = cleanText(homescreenHandler.navigationDestinationName)
        if (name.length > 0)
            return name
        if (coordValid(destinationCoord))
            return shortCoord(destinationCoord)
        return qsTr("Điểm đến")
    }

    function distanceText() {
        var value = cleanText(homescreenHandler.navigationDistanceText)
        return value.length > 0 ? value : qsTr("-- km")
    }

    function routeIdentity(path) {
        if (!path || path.length < 2)
            return ""

        var first = path[0]
        var middle = path[Math.floor(path.length / 2)]
        var last = path[path.length - 1]
        return path.length + ":"
                + Number(first.latitude).toFixed(6) + "," + Number(first.longitude).toFixed(6) + ":"
                + Number(middle.latitude).toFixed(6) + "," + Number(middle.longitude).toFixed(6) + ":"
                + Number(last.latitude).toFixed(6) + "," + Number(last.longitude).toFixed(6)
    }

    function rebuildRoutePath() {
        var path = []
        var source = homescreenHandler.navigationRoutePath
        for (var i = 0; i < source.length; ++i) {
            var coord = toCoordinate(source[i])
            if (coordValid(coord))
                path.push(coord)
        }

        var nextRouteKey = routeIdentity(path)
        var newRoute = nextRouteKey.length > 0
                && (nextRouteKey !== areaMap.routePathKey
                    || areaMap.routePointCount === 0
                    || areaMap.localRouteArrived)
        if (newRoute) {
            areaMap.localRouteArrived = false
            areaMap.routeSessionSeed += 1
            areaMap.routePathKey = nextRouteKey
        }

        if (!areaMap.navigationActive || nextRouteKey.length === 0) {
            clearRoutePath(false, false)
            return
        }

        if (areaMap.routeArrived) {
            clearRoutePath(true, false)
            return
        }

        areaMap.routePath = path
        areaMap.routePointCount = path.length
        refreshCamera()
    }

    function clearRoutePath(keepArrived, forceRendererReset) {
        var arrived = !!keepArrived
        var preservedRouteKey = arrived ? areaMap.routePathKey : ""
        var alreadyClear = areaMap.routePointCount === 0
                && (!areaMap.routePath || areaMap.routePath.length === 0)
                && areaMap.localRouteArrived === arrived
                && (arrived || areaMap.routePathKey.length === 0)

        areaMap.routePath = []
        areaMap.routePointCount = 0
        areaMap.routePathKey = preservedRouteKey
        areaMap.localRouteArrived = arrived

        if (!alreadyClear || forceRendererReset)
            areaMap.routeResetSeed += 1

        refreshCamera()
    }

    function refreshCamera() {
        var map = miniMapItem()
        if (!map)
            return

        if (routeVisible) {
            map.bearing = normalizedBearing(homescreenHandler.navigationHeading)
            map.tilt = boundedTilt(62)
            map.zoomLevel = boundedZoom(18.6)
            map.center = guidanceCenter()
            return
        }

        map.bearing = 0
        map.tilt = boundedTilt(58)
        map.zoomLevel = boundedZoom(16.2)
        map.center = currentCoord
    }

    Rectangle {
        id: infoHeader
        x: 0
        y: 0
        width: parent.width
        height: areaMap.headerHeight
        color: "#0b1420"
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#122033" }
            GradientStop { position: 1.0; color: "#07111d" }
        }

        Column {
            x: 22
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - weatherCard.width - 54
            spacing: 4

            Text {
                width: parent.width
                text: areaMap.currentPlaceTitle()
                color: "#f7fbff"
                font.family: mapFont.name
                font.pixelSize: 24
                font.bold: true
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: areaMap.currentPlaceSubtitle()
                color: "#9fb2c7"
                font.pixelSize: 14
                elide: Text.ElideRight
            }

            Text {
                width: parent.width
                text: areaMap.dateText()
                color: "#b8c7d8"
                font.pixelSize: 13
                elide: Text.ElideRight
            }
        }

        Rectangle {
            id: weatherCard
            x: parent.width - width - 18
            anchors.verticalCenter: parent.verticalCenter
            width: 132
            height: 74
            radius: 16
            color: "#172435"
            border.width: 1
            border.color: "#273a52"

            Canvas {
                id: weatherIcon
                x: 12
                width: 34
                height: 34
                anchors.verticalCenter: parent.verticalCenter
                antialiasing: true

                Connections {
                    target: typeof weather === "undefined" ? null : weather
                    function onConditionChanged() {
                        weatherIcon.requestPaint()
                    }
                }

                onPaint: {
                    var ctx = getContext("2d")
                    var kind = areaMap.weatherKind()
                    ctx.clearRect(0, 0, width, height)
                    ctx.lineCap = "round"
                    ctx.lineJoin = "round"

                    if (kind === "rain") {
                        ctx.fillStyle = "#a8c7fa"
                        ctx.beginPath()
                        ctx.arc(14, 15, 8, Math.PI, Math.PI * 2)
                        ctx.arc(22, 15, 7, Math.PI, Math.PI * 2)
                        ctx.arc(18, 12, 9, Math.PI, Math.PI * 2)
                        ctx.lineTo(28, 21)
                        ctx.lineTo(8, 21)
                        ctx.closePath()
                        ctx.fill()
                        ctx.strokeStyle = "#5aa7ff"
                        ctx.lineWidth = 3
                        ctx.beginPath()
                        ctx.moveTo(11, 26)
                        ctx.lineTo(8, 31)
                        ctx.moveTo(19, 26)
                        ctx.lineTo(16, 32)
                        ctx.moveTo(27, 26)
                        ctx.lineTo(24, 31)
                        ctx.stroke()
                        return
                    }

                    if (kind === "cloud") {
                        ctx.fillStyle = "#cbd5e1"
                        ctx.beginPath()
                        ctx.arc(13, 19, 8, Math.PI, Math.PI * 2)
                        ctx.arc(22, 18, 9, Math.PI, Math.PI * 2)
                        ctx.arc(18, 14, 9, Math.PI, Math.PI * 2)
                        ctx.lineTo(30, 24)
                        ctx.lineTo(6, 24)
                        ctx.closePath()
                        ctx.fill()
                        return
                    }

                    ctx.strokeStyle = "#ffd166"
                    ctx.lineWidth = 3
                    for (var i = 0; i < 8; ++i) {
                        var angle = i * Math.PI / 4
                        ctx.beginPath()
                        ctx.moveTo(17 + Math.cos(angle) * 12, 17 + Math.sin(angle) * 12)
                        ctx.lineTo(17 + Math.cos(angle) * 16, 17 + Math.sin(angle) * 16)
                        ctx.stroke()
                    }
                    ctx.fillStyle = "#ffbd2e"
                    ctx.beginPath()
                    ctx.arc(17, 17, 9, 0, Math.PI * 2)
                    ctx.fill()
                }
            }

            Text {
                x: 54
                y: 12
                width: parent.width - x - 12
                text: areaMap.weatherTemperatureText()
                color: "#ffffff"
                font.family: mapFont.name
                font.pixelSize: 26
                font.bold: true
                elide: Text.ElideRight
            }

            Text {
                x: 54
                y: 44
                width: parent.width - x - 12
                text: areaMap.weatherConditionText()
                color: "#b8c7d8"
                font.pixelSize: 12
                maximumLineCount: 1
                elide: Text.ElideRight
            }
        }
    }

    Item {
        id: mapSurface
        x: 0
        y: infoHeader.height
        width: parent.width
        height: parent.height - infoHeader.height
        clip: true

        Rectangle {
            anchors.fill: parent
            color: "#050c14"
        }

        Loader {
            id: mapLoader
            anchors.fill: parent
            active: areaMap.deferredContentActive
            asynchronous: true
            sourceComponent: mapComponent

            onLoaded: areaMap.rebuildRoutePath()
        }

        Component {
            id: mapComponent

            HomeMap3DSurface {
                id: miniMap
                anchors.fill: parent
                center: areaMap.currentCoord
                zoomLevel: 16.2
                tilt: 58
                firstPersonOffsetY: 64
                currentCoordinate: areaMap.currentCoord
                destinationCoordinate: areaMap.routeVisible ? areaMap.destinationCoord : null
                routePath: areaMap.routeVisible ? areaMap.routePath : []
                routeSessionSeed: areaMap.routeSessionSeed
                routeResetSeed: areaMap.routeResetSeed
                navigationActive: areaMap.routeVisible
                driveSessionActive: true//areaMap.routeVisible
                map3dEnabled: true
                onDriveArrived: function(progress) {
                    areaMap.clearRoutePath(true, true)
                }
                goongMapTilesKey: homescreenHandler.goongMapTilesKey.length > 0
                                   ? homescreenHandler.goongMapTilesKey
                                   : fallbackGoongMapTilesKey
                goongStyleUrl: homescreenHandler.goongStyleUrl.length > 0
                               ? homescreenHandler.goongStyleUrl
                               : "https://tiles.goong.io/assets/goong_map_web.json"
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 70
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#cc050c14" }
                GradientStop { position: 1.0; color: "#00050c14" }
            }
        }

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 82
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#00050c14" }
                GradientStop { position: 1.0; color: "#cc050c14" }
            }
        }

        Rectangle {
            id: routePanel
            x: 16
            y: 16
            width: parent.width - 32
            height: 76
            radius: 18
            visible: areaMap.routeVisible
            color: "#f4f8ff"
            border.width: 1
            border.color: "#d6e4f7"

            Rectangle {
                x: 12
                width: 4
                height: parent.height - 24
                anchors.verticalCenter: parent.verticalCenter
                radius: 2
                color: "#1a73e8"
            }

            Column {
                x: 26
                y: 13
                width: parent.width - distanceBadge.width - 58
                spacing: 4

                Text {
                    width: parent.width
                    text: areaMap.destinationTitleText()
                    color: "#172033"
                    font.family: mapFont.name
                    font.pixelSize: 20
                    font.bold: true
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    text: qsTr("Điểm đến")
                    color: "#667085"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                }
            }

            Rectangle {
                id: distanceBadge
                x: parent.width - width - 14
                width: 112
                height: 44
                anchors.verticalCenter: parent.verticalCenter
                radius: 22
                color: "#e7f0ff"
                border.width: 1
                border.color: "#c8ddff"

                Text {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    text: areaMap.distanceText()
                    color: "#1a73e8"
                    font.family: mapFont.name
                    font.pixelSize: 18
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
        }
    }

    BorderColor {}

    Rectangle {
        anchors.fill: parent
        scale: 1.01
        radius: 12
        color: "transparent"
        border.width: 3
        layer.enabled: true
        layer.effect: MultiEffect {
            blurEnabled: true
            blur: 0.6
        }
        BorderColor { borderWidth: 2 }
    }

    Connections {
        target: homescreenHandler
        function onNavigationStateChanged() {
            areaMap.rebuildRoutePath()
        }
    }

    Component.onCompleted: rebuildRoutePath()
}
