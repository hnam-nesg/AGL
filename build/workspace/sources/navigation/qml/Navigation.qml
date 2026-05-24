import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning
import Qt5Compat.GraphicalEffects
import QtQuick.Controls.Basic as Control
import QtQuick.Effects

ApplicationWindow {
    id: view
    width: 850
    height: 670

    Rectangle{
        id: rect_; y:5; width: 860; height: 660

        Background{width_:1280; height: 800; anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter}

        BorderColor{}
        Rectangle {
            anchors.fill: parent
            radius: 10
            color: "transparent"
            border.width: 3
            layer.enabled: true
            layer.effect: MultiEffect {
                blurEnabled: true
                blur:0.6
            }
            BorderColor{borderWidth: 1; c1: "white"; c2: "white"}
        }

        ListModel { id: routeInfoModel }


        function formatTime(sec)
        {
            var totalSeconds = Number(sec)
            if (!isFinite(totalSeconds) || totalSeconds < 0)
                return ""

            var totalMinutes = Math.max(1, Math.round(totalSeconds / 60))
            var hours = Math.floor(totalMinutes / 60)
            var minutes = totalMinutes % 60
            return hours > 0 ? hours + " giờ " + minutes + " phút" : minutes + " phút"
        }

        function formatDistance(meters)
        {
            var dist = Number(meters)
            if (!isFinite(dist) || dist < 0)
                return ""

            if (dist >= 1000) {
                var km = dist / 1000
                return (km >= 100 ? Math.round(km) : Math.round(km * 10) / 10) + " km"
            }

            return Math.round(dist) + " m"
        }

        function stripInstructionMarkup(instruction)
        {
            var text = String(instruction || "")
            text = text.replace(/<[^>]+>/g, " ")
            text = text.replace(/&nbsp;/g, " ")
            text = text.replace(/&amp;/g, "&")
            text = text.replace(/&lt;/g, "<")
            text = text.replace(/&gt;/g, ">")
            text = text.replace(/\s+/g, " ")
            return text.trim()
        }

        function realisticTravelTime(rawSeconds, meters)
        {
            var raw = Number(rawSeconds)
            var dist = Number(meters)
            var hasRaw = isFinite(raw) && raw > 0
            var hasDistance = isFinite(dist) && dist > 0

            if (!hasRaw && !hasDistance)
                return Number.NaN

            if (!hasDistance)
                return raw * 1.45

            var km = dist / 1000.0
            var averageSpeedKmh = 22
            if (km <= 1.2)
                averageSpeedKmh = 12
            else if (km <= 5)
                averageSpeedKmh = 17
            else if (km <= 15)
                averageSpeedKmh = 22
            else if (km <= 35)
                averageSpeedKmh = 28
            else
                averageSpeedKmh = 36

            var speedBased = dist / (averageSpeedKmh * 1000 / 3600)
            var urbanBuffer = Math.min(900, Math.max(90, km * 55))
            var realistic = speedBased + urbanBuffer

            if (hasRaw)
                realistic = Math.max(realistic, raw * 1.35)

            var roundedMinutes = Math.max(1, Math.ceil(realistic / 60 / 2) * 2)
            return roundedMinutes * 60
        }

        function routeTravelTimeSeconds(route)
        {
            if (!route)
                return Number.NaN

            var raw = Number(route.travelTime)
            if (route.provider === "goong" && isFinite(raw) && raw > 0)
                return raw

            return rect_.realisticTravelTime(route.travelTime, route.distance)
        }

        function clearRouteInfo(message) {
            routeInfoModel.clear()
            root.nextGuideText = qsTr("")
            root.nextGuideIcon = "images/icons8-arrow-up-94.png"
            root.displayTravelTimeSeconds = Number.NaN
            ins.contentText = message || qsTr("")
            distance_target.contentText = qsTr("")
            time.contentText = qsTr("")
            distance.contentText = qsTr("")
            time_estimate.contentText = qsTr("")
            image_turn.source = "images/icons8-arrow-up-94.png"
            root.publishRouteState()
        }

        function turnIcon(instruction) {
            if (/(u-turn|quay đầu|turn left|slight left|sharp left|keep left|bear left|fork left|left|trái)/i.test(instruction))
                return "images/icons8-undo-94.png"
            if (/(turn right|slight right|sharp right|keep right|bear right|fork right|right|phải)/i.test(instruction))
                return "images/icons8-redo-94.png"
            return "images/icons8-arrow-up-94.png"
        }

        function updateEstimate() {
            var travelTime = root.displayTravelTimeSeconds
            if (isFinite(travelTime) && travelTime > 0) {
                time_estimate.contentText = Qt.formatTime(new Date(Date.now() + travelTime * 1000), "hh:mm")
                return
            }

            var route = root.activeRouteData
            if (!route && routeModel.count > 0)
                route = routeModel.get(0)

            if (!route) {
                time_estimate.contentText = qsTr("")
                return
            }

            travelTime = rect_.routeTravelTimeSeconds(route)
            time_estimate.contentText = isFinite(travelTime)
                    ? Qt.formatTime(new Date(Date.now() + travelTime * 1000), "hh:mm")
                    : qsTr("")
        }

        function routeError(message) {
            console.warn("Route error:", message || "")
            root.activeRouteData = null
            root.usingFallbackRoute = false
            root.hasRoute = false
            root.guidanceActive = false
            fallbackLine.path = []
            clearRouteInfo(message || qsTr("Không tìm thấy tuyến đường"))
        }

        function localizeInstruction(instruction) {
            var text = stripInstructionMarkup(instruction || "")
            if (text.length === 0)
                return qsTr("Tiếp tục")

            text = text.replace(/You have reached your destination/ig, qsTr("Bạn đã đến nơi"))
            text = text.replace(/Arrive at your destination/ig, qsTr("Đến điểm đến"))
            text = text.replace(/Head ([a-z ]+) on/ig, qsTr("Đi theo hướng đó vào"))
            text = text.replace(/Continue on/ig, qsTr("Tiếp tục đi trên"))
            text = text.replace(/Continue/ig, qsTr("Tiếp tục"))
            text = text.replace(/Turn slight left/ig, qsTr("Rẽ hơi trái"))
            text = text.replace(/Turn slight right/ig, qsTr("Rẽ hơi phải"))
            text = text.replace(/Turn sharp left/ig, qsTr("Rẽ gắt trái"))
            text = text.replace(/Turn sharp right/ig, qsTr("Rẽ gắt phải"))
            text = text.replace(/Turn left/ig, qsTr("Rẽ trái"))
            text = text.replace(/Turn right/ig, qsTr("Rẽ phải"))
            text = text.replace(/Keep left/ig, qsTr("Đi bên trái"))
            text = text.replace(/Keep right/ig, qsTr("Đi bên phải"))
            text = text.replace(/Make a U-turn/ig, qsTr("Quay đầu"))
            text = text.replace(/At the roundabout/ig, qsTr("Tại vòng xuyến"))
            text = text.replace(/take the/ig, qsTr("đi theo"))
            text = text.replace(/Take/ig, qsTr("Đi theo"))
            text = text.replace(/exit/ig, qsTr("lối ra"))
            text = text.replace(/onto/ig, qsTr("vào"))
            text = text.replace(/Head/ig, qsTr("Đi"))
            text = text.replace(/northwest|northeast|southwest|southeast|north|south|east|west/ig, "")
            text = text.replace(/left/ig, qsTr("trái"))
            text = text.replace(/right/ig, qsTr("phải"))
            return text
        }

        function contentRoute(){
            routeInfoModel.clear()
            var route = root.activeRouteData
            if (!route && routeModel.count > 0)
                route = routeModel.get(0)

            if (!route) {
                routeError(qsTr("Không tìm thấy tuyến đường"))
                return false
            }

            var segments = route.segments || []
            if (segments.length === 0) {
                routeError(qsTr("Không có hướng dẫn tuyến đường"))
                return false
            }

            for (var i = 0; i < segments.length; i++) {
                var maneuver = segments[i].maneuver || {}
                var localized = rect_.localizeInstruction(maneuver.instructionText || "")
                routeInfoModel.append({
                    "instruction": localized,
                    "distance": rect_.formatDistance(maneuver.distanceToNextInstruction),
                    "icon": rect_.turnIcon(localized)
                })
            }

            var firstManeuver = segments[0].maneuver || {}
            var nextManeuver = segments.length > 1 ? (segments[1].maneuver || {}) : firstManeuver
            var firstInstruction = firstManeuver.instructionText || qsTr("Tiếp tục")
            var nextInstruction = nextManeuver.instructionText || firstManeuver.instructionText
            ins.contentText = rect_.localizeInstruction(firstInstruction)
            root.nextGuideText = rect_.localizeInstruction(nextInstruction || "")
            root.nextGuideIcon = rect_.turnIcon(nextInstruction || "")
            distance_target.contentText = rect_.formatDistance(firstManeuver.distanceToNextInstruction)
            root.displayTravelTimeSeconds = rect_.routeTravelTimeSeconds(route)
            time.contentText = rect_.formatTime(root.displayTravelTimeSeconds)
            distance.contentText = rect_.formatDistance(route.distance)
            image_turn.source = rect_.turnIcon(firstInstruction)
            rect_.updateEstimate()
            root.publishRouteState()
            return true
        }

            function fitRoute() {
                var start = root.startCoord
                var dest = root.destCoord
                if (!start || !dest)
                    return

            var minLat = Math.min(start.latitude, dest.latitude)
            var maxLat = Math.max(start.latitude, dest.latitude)
            var minLon = Math.min(start.longitude, dest.longitude)
            var maxLon = Math.max(start.longitude, dest.longitude)

            var centerLat = (minLat + maxLat) / 2
            var centerLon = (minLon + maxLon) / 2

            mapView.map.center = QtPositioning.coordinate(centerLat, centerLon)

            var latDiff = maxLat - minLat
            var lonDiff = maxLon - minLon

            latDiff *= 1.4
            lonDiff *= 1.4

            var maxDiff = Math.max(latDiff, lonDiff)
            var zoomLevel = 10

            if (maxDiff > 30) zoomLevel = 4
            else if (maxDiff > 15) zoomLevel = 6
            else if (maxDiff > 8) zoomLevel = 7
            else if (maxDiff > 4) zoomLevel = 8
            else if (maxDiff > 2) zoomLevel = 9
            else if (maxDiff > 1) zoomLevel = 10
            else if (maxDiff > 0.5) zoomLevel = 11
            else if (maxDiff > 0.2) zoomLevel = 12
            else if (maxDiff > 0.1) zoomLevel = 13
            else if (maxDiff > 0.02) zoomLevel = 15
            else if (maxDiff > 0.01) zoomLevel = 16
            else zoomLevel = 17

            mapView.followExternalCenter = true
            mapView.map.zoomLevel = root.map3dEnabled ? root.boundedZoom(zoomLevel + root.previewZoomBoost) : zoomLevel
            root.applyPreviewCamera(true)
        }

        Item{
            id: root
            x: 20; y: 20; width: 810; height: 620//anchors.fill: parent
            clip: true

            property var sampleCoord: QtPositioning.coordinate(10.80690, 106.70290)
            property string sampleLocationName: "125/16/11 Bùi Đình Túy, Bình Thạnh, TP.HCM"
            property var defaultCoord: sampleCoord
            property var startCoord: defaultCoord
            property var destCoord: defaultCoord
            property bool hasStartPlace: true
            property bool hasDestPlace: false
            property string startName: sampleLocationName
            property string destName: ""
            property string nextGuideText: ""
            property real displayTravelTimeSeconds: Number.NaN
            property var activeRouteData: null
            property bool usingFallbackRoute: false
            property bool hasRoute: false
            property bool gpsFixReceived: false
            property bool guidanceActive: false
            property bool followPosition: true
            property bool manualMapGestureActive: false
            property bool map3dEnabled: true
            property real previewTilt: 52
            property real previewZoom: 16.4
            property real navigationTilt: 60
            property real navigationZoom: 18.2
            property real previewZoomBoost: 0.7
            property real guidanceMarkerOffsetMeters: 58
            property string nextGuideIcon: "images/icons8-arrow-up-94.png"

            function coordValid(coord) {
                return coord
                        && isFinite(coord.latitude) && isFinite(coord.longitude)
                        && coord.latitude >= -90 && coord.latitude <= 90
                        && coord.longitude >= -180 && coord.longitude <= 180
                        && !(Math.abs(coord.latitude) < 0.000001 && Math.abs(coord.longitude) < 0.000001)
            }

            PositionSource {
                id: ps
                active: true
                updateInterval: 1000
                onPositionChanged: {
                    if (!root.coordValid(position.coordinate))
                        return

                    root.gpsFixReceived = true
                    if (position.directionValid)
                    root.currentHeading = root.normalizedBearing(position.direction)
                    if (searchOverlay.visible && !root.hasStartPlace)
                        mapView.map.center = position.coordinate
                    if (root.guidanceActive && root.followPosition)
                        root.applyGuidanceCamera(false)
                    if (searchOverlay.dbusNavigationActive && root.hasDestPlace && !root.hasStartPlace)
                        searchOverlay.useGpsStart(true)
                    mapView.scheduleSync()
                }
            }
            property bool hasRealGps: gpsFixReceived && ps.valid && root.coordValid(ps.position.coordinate)
            property bool hasLivePos: true
            property var gpsCoord: root.hasRealGps ? ps.position.coordinate : root.sampleCoord
            property var currentCoord: root.hasLivePos ? root.gpsCoord : root.startCoord
            property var searchBiasCoord: root.hasLivePos ? root.gpsCoord : (root.hasStartPlace ? root.startCoord : root.defaultCoord)
            property real currentHeading: 0

            function currentLocationName() {
                return root.hasRealGps ? qsTr("Vị trí định vị hiện tại") : root.sampleLocationName
            }

            function guidanceCoord() {
                if (root.hasRealGps)
                    return root.gpsCoord
                if (root.hasStartPlace)
                    return root.startCoord
                return root.currentCoord
            }

            function vehicleHeading() {
                if (root.hasRealGps && isFinite(root.currentHeading))
                    return root.normalizedBearing(root.currentHeading)

                var routeBearing = root.routeBearingAt(root.guidanceCoord())
                if (isFinite(routeBearing))
                    return routeBearing
                if (root.hasStartPlace && root.hasDestPlace)
                    return root.bearingBetween(root.startCoord, root.destCoord)
                return isFinite(mapView.map.bearing) ? mapView.map.bearing : 0
            }

            function guidanceBearing() {
                return root.vehicleHeading()
            }

            function markerBearing() {
                return root.guidanceActive ? root.vehicleHeading() : root.currentHeading
            }

            function moveAhead(coord, bearingDeg, distM) {
                if (!root.coordValid(coord))
                    return coord

                const r = 6371000.0
                const toRad = x => x * Math.PI / 180
                const toDeg = x => x * 180 / Math.PI
                const br = toRad(bearingDeg)
                const lat1 = toRad(coord.latitude)
                const lon1 = toRad(coord.longitude)
                const d = distM / r
                const lat2 = Math.asin(Math.sin(lat1) * Math.cos(d)
                                      + Math.cos(lat1) * Math.sin(d) * Math.cos(br))
                const lon2 = lon1 + Math.atan2(Math.sin(br) * Math.sin(d) * Math.cos(lat1),
                                                Math.cos(d) - Math.sin(lat1) * Math.sin(lat2))
                return QtPositioning.coordinate(toDeg(lat2), toDeg(lon2))
            }

            function guidanceCenter() {
                return root.moveAhead(root.guidanceCoord(),
                                      root.normalizedBearing(root.guidanceBearing() + 180),
                                      root.guidanceMarkerOffsetMeters)
            }

            function activeRoutePath() {
                if (root.activeRouteData && root.activeRouteData.path && root.activeRouteData.path.length > 1)
                    return root.activeRouteData.path
                if (root.usingFallbackRoute && fallbackLine.path && fallbackLine.path.length > 1)
                    return fallbackLine.path
                return []
            }

            function routePathJson() {
                var path = root.activeRoutePath()
                var points = []
                for (var i = 0; i < path.length; ++i) {
                    if (root.coordValid(path[i]))
                        points.push([path[i].latitude, path[i].longitude])
                }
                return JSON.stringify(points)
            }

            function publishRouteState() {
                var active = root.hasRoute && root.hasDestPlace
                var current = active ? root.guidanceCoord() : root.currentCoord
                var start = root.hasStartPlace ? root.startCoord : root.currentCoord
                var dest = root.hasDestPlace ? root.destCoord : root.currentCoord

                NavigationDBusService.publishRouteState(active,
                                                        root.coordValid(current) ? current.latitude : Number.NaN,
                                                        root.coordValid(current) ? current.longitude : Number.NaN,
                                                        root.coordValid(start) ? start.latitude : Number.NaN,
                                                        root.coordValid(start) ? start.longitude : Number.NaN,
                                                        root.coordValid(dest) ? dest.latitude : Number.NaN,
                                                        root.coordValid(dest) ? dest.longitude : Number.NaN,
                                                        active ? root.markerBearing() : 0,
                                                        active ? ins.contentText : qsTr(""),
                                                        active ? root.nextGuideText : qsTr(""),
                                                        active ? distance.contentText : qsTr(""),
                                                        active ? time.contentText : qsTr(""),
                                                        active ? root.routePathJson() : "[]",
                                                        active ? root.destName : qsTr(""))
            }

            function routeBearingAt(coord) {
                if (!root.coordValid(coord))
                    return Number.NaN

                var path = root.activeRoutePath()
                if (!path || path.length < 2)
                    return Number.NaN

                var bestIndex = 0
                var bestDistance = Number.POSITIVE_INFINITY
                for (var i = 0; i < path.length - 1; ++i) {
                    var d = root.routeDistanceMeters(coord, path[i])
                    if (d < bestDistance) {
                        bestDistance = d
                        bestIndex = i
                    }
                }

                var from = path[bestIndex]
                for (var j = bestIndex + 1; j < path.length; ++j) {
                    if (root.routeDistanceMeters(from, path[j]) > 8)
                        return root.bearingBetween(from, path[j])
                }

                if (path.length >= 2)
                    return root.bearingBetween(path[path.length - 2], path[path.length - 1])
                return Number.NaN
            }

            function applyGuidanceCamera(snap) {
                if (!root.guidanceActive)
                    return

                var oldManualState = root.manualMapGestureActive
                if (snap)
                    root.manualMapGestureActive = true

                mapView.followExternalCenter = true
                mapView.map.tilt = root.map3dEnabled ? root.navigationTilt : 0
                mapView.map.bearing = root.guidanceBearing()
                mapView.map.zoomLevel = root.navigationZoom
                mapView.map.center = root.guidanceCenter()

                if (snap)
                    root.manualMapGestureActive = oldManualState

                root.publishRouteState()
            }

            function routeDistanceMeters(a, b) {
                if (!root.coordValid(a) || !root.coordValid(b))
                    return Number.POSITIVE_INFINITY

                const toRad = x => x * Math.PI / 180
                const r = 6371000.0
                const dLat = toRad(b.latitude - a.latitude)
                const dLon = toRad(b.longitude - a.longitude)
                const lat1 = toRad(a.latitude)
                const lat2 = toRad(b.latitude)
                const h = Math.sin(dLat / 2) * Math.sin(dLat / 2)
                        + Math.cos(lat1) * Math.cos(lat2)
                        * Math.sin(dLon / 2) * Math.sin(dLon / 2)
                const clamped = Math.max(0, Math.min(1, h))
                return r * 2 * Math.atan2(Math.sqrt(clamped), Math.sqrt(1 - clamped))
            }

            function routeMatchesCurrent(route) {
                if (!route)
                    return false

                const path = route.path || []
                if (!path || path.length < 2)
                    return true

                const startError = root.routeDistanceMeters(path[0], root.startCoord)
                const destError = root.routeDistanceMeters(path[path.length - 1], root.destCoord)
                return startError < 600 && destError < 600
            }

            Plugin {
                id: osm
                name: "osm"
                //PluginParameter { name: "osm.useragent"; value: "IVI_AGL/1.0/hoainam16119@gmail.com" }
                PluginParameter { name: "osm.mapping.host"; value: "https://tile.openstreetmap.org" }
                PluginParameter { name: "osm.mapping.providersrepository.enabled"; value: true }
                PluginParameter { name: "osm.mapping.highdpi_tiles"; value: true }
                PluginParameter { name: "osm.mapping.providersrepository.address"; value: "http://127.0.0.1:8000/" }
                PluginParameter { name: "osm.routing.osrm.server";  value: "" }
                PluginParameter { name: "osm.routing.osrm.apiversion"; value: "v1" }
                PluginParameter { name: "osm.routing.profile"; value: "car" }
            }

            function bearingBetween(a,b){
                const toRad = x=>x*Math.PI/180, toDeg = x=>x*180/Math.PI
                const la1=toRad(a.latitude), la2=toRad(b.latitude)
                const dLon=toRad(b.longitude - a.longitude)
                const y=Math.sin(dLon)*Math.cos(la2)
                const x=Math.cos(la1)*Math.sin(la2)-Math.sin(la1)*Math.cos(la2)*Math.cos(dLon)
                let br=toDeg(Math.atan2(y,x)); if (br<0) br+=360; return br
            }
            function wrap180(d){ d=(d+540)%360-180; return d } // [-180,180]

            function angleRouteVsScreenY(routeA, routeB){
                const rb = bearingBetween(routeA, routeB)   // bearing đoạn route (deg)
                const mb = mapView.map.bearing                      // bearing của map (deg)
                // 0° = song song trục Y hướng lên; +90° = nghiêng về phải, -90° = nghiêng về trái
                return wrap180(rb - mb)
            }

            function boundedZoom(value) {
                var minZoom = isFinite(mapView.map.minimumZoomLevel) ? mapView.map.minimumZoomLevel : 2
                var maxZoom = isFinite(mapView.map.maximumZoomLevel) ? mapView.map.maximumZoomLevel : 20
                return Math.max(minZoom, Math.min(maxZoom, value))
            }

            function boundedTilt(value) {
                var minTilt = isFinite(mapView.map.minimumTilt) ? mapView.map.minimumTilt : 0
                var maxTilt = isFinite(mapView.map.maximumTilt) ? mapView.map.maximumTilt : 60
                return Math.max(minTilt, Math.min(maxTilt, value))
            }

            function preferredPreviewTilt() {
                return root.map3dEnabled ? root.boundedTilt(root.previewTilt) : 0
            }

            function applyPreviewCamera(resetBearing) {
                if (root.guidanceActive)
                    return

                mapView.map.tilt = root.preferredPreviewTilt()
                if (resetBearing)
                    mapView.map.bearing = 0
            }

            function toggleMap3d() {
                root.map3dEnabled = !root.map3dEnabled
                if (root.guidanceActive)
                    root.applyGuidanceCamera(true)
                else
                    root.applyPreviewCamera(false)
            }

            function normalizedBearing(value) {
                var bearing = value % 360
                return bearing < 0 ? bearing + 360 : bearing
            }

            MapLibreSurface {
                id: mapView
                anchors.fill: parent
                centerCoordinate: root.searchBiasCoord
                currentCoordinate: root.guidanceActive ? root.guidanceCoord()
                                   : (root.hasLivePos && !root.hasStartPlace ? root.gpsCoord : root.startCoord)
                destinationCoordinate: root.hasDestPlace ? root.destCoord : null
                routePath: root.activeRoutePath()
                zoomLevel: root.guidanceActive ? root.navigationZoom : root.previewZoom
                pitch: root.guidanceActive ? root.navigationTilt : root.previewTilt
                bearing: root.guidanceActive ? root.guidanceBearing() : 0
                map3dEnabled: root.map3dEnabled
                rendererUrl: "qrc:/goong-map.html"
                goongMapTilesKey: "Xgw2Eiqb38KmKuSsxQFH5c4NtEuORfbAdWfbxXIB"
                goongStyleUrl: GoongStyleUrl
                buildingSourceLayer: "VN_Building"
                // Source map cu OpenMapTiles local van giu lai de rollback neu can:
                // rendererUrl: "qrc:/maplibre.html"
                // vectorTileUrl: "http://127.0.0.1:8080/data/v3.json"
                // buildingSourceLayer: "building"
                onUserGestureStarted: {
                    root.manualMapGestureActive = true
                    root.followPosition = false
                    mapView.followExternalCenter = false
                    root.manualMapGestureActive = false
                }

                Behavior on height{NumberAnimation{duration: 200; easing.type: Easing.Linear}}
            }

            MouseArea {
                id: mapPanArea
                anchors.fill: mapView
                z: 1
                enabled: false
                acceptedButtons: Qt.LeftButton
                preventStealing: false
                hoverEnabled: false
                property real lastX: 0
                property real lastY: 0

                onPressed: function(mouse) {
                    if (mapTouchArea.twoFingerActive)
                        return
                    root.manualMapGestureActive = true
                    root.followPosition = false
                    mapView.followExternalCenter = false
                    lastX = mouse.x
                    lastY = mouse.y
                }

                onPositionChanged: function(mouse) {
                    if (!mapPanArea.pressed || mapTouchArea.twoFingerActive)
                        return
                    mapView.panBy(mouse.x - lastX, mouse.y - lastY)
                    lastX = mouse.x
                    lastY = mouse.y
                }

                onReleased: root.manualMapGestureActive = false
                onCanceled: root.manualMapGestureActive = false

                onWheel: function(wheel) {
                    root.manualMapGestureActive = true
                    root.followPosition = false
                    mapView.followExternalCenter = false
                    mapView.zoomBy(wheel.angleDelta.y / 480)
                    root.manualMapGestureActive = false
                }
            }

            MultiPointTouchArea {
                id: mapTouchArea
                anchors.fill: mapView
                z: 1.1
                enabled: false
                mouseEnabled: false
                minimumTouchPoints: 2
                maximumTouchPoints: 2
                touchPoints: [
                    TouchPoint { id: mapTouchPoint1 },
                    TouchPoint { id: mapTouchPoint2 }
                ]

                property bool twoFingerActive: false
                property real startZoom: 14
                property real startBearing: 0
                property real startTilt: 0
                property real startDistance: 1
                property real startAngle: 0
                property real startMidY: 0

                function pointDistance() {
                    var dx = mapTouchPoint2.x - mapTouchPoint1.x
                    var dy = mapTouchPoint2.y - mapTouchPoint1.y
                    return Math.max(1, Math.sqrt(dx * dx + dy * dy))
                }

                function pointAngle() {
                    return Math.atan2(mapTouchPoint2.y - mapTouchPoint1.y,
                                      mapTouchPoint2.x - mapTouchPoint1.x) * 180 / Math.PI
                }

                function pointMidY() {
                    return (mapTouchPoint1.y + mapTouchPoint2.y) / 2
                }

                function beginTwoFingerGesture() {
                    root.followPosition = false
                    mapView.followExternalCenter = false
                    root.manualMapGestureActive = true
                    twoFingerActive = true
                    startZoom = mapView.map.zoomLevel
                    startBearing = mapView.map.bearing
                    startTilt = mapView.map.tilt
                    startDistance = pointDistance()
                    startAngle = pointAngle()
                    startMidY = pointMidY()
                }

                function finishTwoFingerGesture() {
                    twoFingerActive = false
                    root.manualMapGestureActive = false
                }

                onGestureStarted: function(gesture) {
                    if (!keyboardPanel.open)
                        gesture.grab()
                }

                onPressed: function(points) {
                    if (keyboardPanel.open || points.length < 2)
                        return
                    beginTwoFingerGesture()
                }

                onUpdated: function(points) {
                    if (!twoFingerActive || keyboardPanel.open || points.length < 2)
                        return

                    var scale = Math.max(0.25, Math.min(4.0, pointDistance() / startDistance))
                    var rotationDelta = root.wrap180(pointAngle() - startAngle)
                    mapView.map.zoomLevel = root.boundedZoom(startZoom + Math.log(scale) / Math.LN2)
                    if (Math.abs(rotationDelta) > 1.5)
                        mapView.map.bearing = root.normalizedBearing(startBearing - rotationDelta)
                    mapView.map.tilt = root.boundedTilt(startTilt - (pointMidY() - startMidY) * 0.14)
                }

                onReleased: function(points) {
                    if (!mapTouchPoint1.pressed || !mapTouchPoint2.pressed)
                        finishTwoFingerGesture()
                }

                onCanceled: finishTwoFingerGesture()
            }

            QtObject {
                id: fallbackLine
                property var path: []
            }

            RouteQuery { id: routeQuery }
            RouteModel {
                id: routeModel
                plugin: osm
                query: routeQuery
                onStatusChanged: {
                    if (status == RouteModel.Ready) {
                        routeCtl.busy = false

                        if (count === 0) {
                            root.activeRouteData = null
                            root.hasRoute = false
                            rect_.routeError(qsTr("Không tìm thấy tuyến đường"))
                            httpRoute.drawCarRoute(root.startCoord, root.destCoord)
                        } else {
                            const nextRoute = routeModel.get(0)
                            if (!root.routeMatchesCurrent(nextRoute)) {
                                console.warn("Ignoring stale RouteModel route")
                                if (fallbackLine.path.length > 0) {
                                    root.hasRoute = true
                                    root.usingFallbackRoute = true
                                }
                                return
                            }

                            root.usingFallbackRoute = false
                            root.activeRouteData = nextRoute
                            root.hasRoute = true
                            fallbackLine.path = []
                            rect_.contentRoute()
                            if (root.guidanceActive) {
                                root.applyGuidanceCamera(false)
                            } else {
                                rect_.fitRoute()
                            }
                        }
                    } else if (status == RouteModel.Error) {
                        routeCtl.busy = false
                        root.activeRouteData = null
                        root.hasRoute = false
                        rect_.routeError(qsTr("Dịch vụ dẫn đường không khả dụng"))
                        httpRoute.drawCarRoute(root.startCoord, root.destCoord)
                    }
                }
            }

            QtObject {
                id: httpRoute
                property int requestSeq: 0
                property int activeGoongRequestId: 0

                function decodePolyline(encoded) {
                    var points = []
                    if (!encoded || encoded.length === 0)
                        return points

                    var index = 0
                    var lat = 0
                    var lng = 0

                    while (index < encoded.length) {
                        var b
                        var shift = 0
                        var result = 0
                        do {
                            b = encoded.charCodeAt(index++) - 63
                            result |= (b & 0x1f) << shift
                            shift += 5
                        } while (b >= 0x20 && index < encoded.length)
                        var dlat = (result & 1) ? ~(result >> 1) : (result >> 1)
                        lat += dlat

                        shift = 0
                        result = 0
                        do {
                            b = encoded.charCodeAt(index++) - 63
                            result |= (b & 0x1f) << shift
                            shift += 5
                        } while (b >= 0x20 && index < encoded.length)
                        var dlng = (result & 1) ? ~(result >> 1) : (result >> 1)
                        lng += dlng

                        points.push(QtPositioning.coordinate(lat / 100000.0, lng / 100000.0))
                    }

                    return points
                }

                function pointsFromRoute(route) {
                    if (!route)
                        return []

                    var encoded = ""
                    if (route.overview_polyline && route.overview_polyline.points)
                        encoded = route.overview_polyline.points
                    else if (route.polyline)
                        encoded = route.polyline
                    else if (route.geometry && typeof route.geometry === "string")
                        encoded = route.geometry

                    if (encoded.length > 0)
                        return decodePolyline(encoded)

                    if (route.geometry && route.geometry.coordinates) {
                        var coords = route.geometry.coordinates
                        var pts = []
                        for (var i = 0; i < coords.length; ++i)
                            appendPoint(pts, QtPositioning.coordinate(coords[i][1], coords[i][0]))
                        return pts
                    }

                    if (route.legs) {
                        var stepPts = []
                        for (var legIndex = 0; legIndex < route.legs.length; ++legIndex) {
                            var leg = route.legs[legIndex] || {}
                            var steps = leg.steps || []
                            for (var stepIndex = 0; stepIndex < steps.length; ++stepIndex) {
                                var step = steps[stepIndex] || {}
                                var stepEncoded = stepPolyline(step)
                                if (stepEncoded.length > 0) {
                                    var decoded = decodePolyline(stepEncoded)
                                    for (var j = 0; j < decoded.length; ++j)
                                        appendPoint(stepPts, decoded[j])
                                } else if (step.geometry && step.geometry.coordinates) {
                                    var stepCoords = step.geometry.coordinates
                                    for (var k = 0; k < stepCoords.length; ++k)
                                        appendPoint(stepPts, QtPositioning.coordinate(stepCoords[k][1], stepCoords[k][0]))
                                }
                            }
                        }
                        if (stepPts.length > 1)
                            return stepPts
                    }

                    return []
                }

                function routeDistance(route) {
                    if (!route)
                        return Number.NaN
                    var direct = metricValue(route.distance)
                    if (isFinite(direct))
                        return direct
                    if (route.legs && route.legs.length > 0) {
                        var total = 0
                        var found = false
                        for (var i = 0; i < route.legs.length; ++i) {
                            var legDistance = metricValue(route.legs[i].distance)
                            if (isFinite(legDistance)) {
                                total += legDistance
                                found = true
                            }
                        }
                        if (found)
                            return total
                    }
                    return Number.NaN
                }

                function routeDuration(route) {
                    if (!route)
                        return Number.NaN
                    var direct = durationValue(route.duration)
                    if (isFinite(direct))
                        return direct
                    if (route.legs && route.legs.length > 0) {
                        var total = 0
                        var found = false
                        for (var i = 0; i < route.legs.length; ++i) {
                            var legDuration = durationValue(route.legs[i].duration)
                            if (isFinite(legDuration)) {
                                total += legDuration
                                found = true
                            }
                        }
                        if (found)
                            return total
                    }
                    return Number.NaN
                }

                function metricValue(value) {
                    if (value === undefined || value === null)
                        return Number.NaN
                    if (typeof value === "number")
                        return Number(value)
                    if (typeof value === "object") {
                        if (isFinite(Number(value.value)))
                            return Number(value.value)
                        if (isFinite(Number(value.distance)))
                            return Number(value.distance)
                        if (value.text !== undefined)
                            return metricValue(value.text)
                    }

                    var text = String(value).replace(",", ".").toLowerCase()
                    var match = text.match(/([0-9]+(?:\.[0-9]+)?)\s*(km|kilometer|kilometre|m|meter|metre)?/)
                    if (!match)
                        return Number.NaN

                    var number = Number(match[1])
                    if (!isFinite(number))
                        return Number.NaN
                    var unit = match[2] || "m"
                    return unit.indexOf("km") === 0 || unit.indexOf("kilo") === 0 ? number * 1000 : number
                }

                function durationValue(value) {
                    if (value === undefined || value === null)
                        return Number.NaN
                    if (typeof value === "number")
                        return Number(value)
                    if (typeof value === "object") {
                        if (isFinite(Number(value.value)))
                            return Number(value.value)
                        if (isFinite(Number(value.duration)))
                            return Number(value.duration)
                        if (value.text !== undefined)
                            return durationValue(value.text)
                    }

                    var text = String(value).toLowerCase()
                    var total = 0
                    var found = false
                    var hour = text.match(/([0-9]+(?:[\.,][0-9]+)?)\s*(h|giờ|gio|hour)/)
                    if (hour) {
                        total += Number(hour[1].replace(",", ".")) * 3600
                        found = true
                    }
                    var minute = text.match(/([0-9]+(?:[\.,][0-9]+)?)\s*(min|phút|phut|minute)/)
                    if (minute) {
                        total += Number(minute[1].replace(",", ".")) * 60
                        found = true
                    }
                    var second = text.match(/([0-9]+(?:[\.,][0-9]+)?)\s*(s|giây|giay|second)/)
                    if (second) {
                        total += Number(second[1].replace(",", "."))
                        found = true
                    }
                    return found ? total : Number.NaN
                }

                function stepPolyline(step) {
                    if (!step)
                        return ""
                    if (step.polyline && step.polyline.points)
                        return step.polyline.points
                    if (typeof step.polyline === "string")
                        return step.polyline
                    if (step.geometry && typeof step.geometry === "string")
                        return step.geometry
                    return ""
                }

                function stepInstruction(step, index, count) {
                    if (!step)
                        return index === count - 1 ? qsTr("Đến điểm đến") : qsTr("Tiếp tục")

                    var maneuver = step.maneuver || {}
                    var text = step.html_instructions || step.instruction || step.instructions
                            || maneuver.instructionText || maneuver.instruction || maneuver.type
                            || step.name || ""
                    text = rect_.stripInstructionMarkup(text)
                    if (text.length === 0) {
                        if (index === 0)
                            text = qsTr("Bắt đầu đi")
                        else if (index === count - 1)
                            text = qsTr("Đến điểm đến")
                        else
                            text = qsTr("Tiếp tục")
                    }
                    return text
                }

                function routeSteps(route) {
                    var segments = []
                    if (route && route.legs) {
                        for (var legIndex = 0; legIndex < route.legs.length; ++legIndex) {
                            var leg = route.legs[legIndex] || {}
                            var steps = leg.steps || []
                            for (var i = 0; i < steps.length; ++i) {
                                var step = steps[i] || {}
                                segments.push({
                                    maneuver: {
                                        instructionText: stepInstruction(step, segments.length, steps.length),
                                        distanceToNextInstruction: metricValue(step.distance)
                                    }
                                })
                            }
                        }
                    }

                    if (segments.length === 0) {
                        segments.push({
                            maneuver: {
                                instructionText: qsTr("Đi theo tuyến đường Goong"),
                                distanceToNextInstruction: routeDistance(route)
                            }
                        })
                    } else {
                        var lastManeuver = segments[segments.length - 1].maneuver
                        if (!lastManeuver.instructionText || lastManeuver.instructionText === qsTr("Tiếp tục"))
                            lastManeuver.instructionText = qsTr("Đến điểm đến")
                    }

                    return segments
                }

                function appendPoint(points, point) {
                    if (!point)
                        return
                    if (points.length > 0) {
                        var last = points[points.length - 1]
                        if (Math.abs(last.latitude - point.latitude) < 0.000001
                                && Math.abs(last.longitude - point.longitude) < 0.000001)
                            return
                    }
                    points.push(point)
                }

                function routePathDistance(points) {
                    if (!points || points.length < 2)
                        return Number.NaN
                    var total = 0
                    for (var i = 1; i < points.length; ++i)
                        total += root.routeDistanceMeters(points[i - 1], points[i])
                    return total
                }

                function drawCarRoute(start, dest) {
                    if (!start || !dest)
                        return

                    const seq = ++requestSeq
                    fallbackLine.path = []
                    root.usingFallbackRoute = false
                    root.hasRoute = false

                    if (!GoongRestClient.available) {
                        rect_.routeError(qsTr("Thiếu GOONG_REST_API_KEY"))
                        return
                    }

                    activeGoongRequestId = GoongRestClient.direction(start.latitude,
                                                                     start.longitude,
                                                                     dest.latitude,
                                                                     dest.longitude,
                                                                     "car")
                    if (activeGoongRequestId <= 0 || activeGoongRequestId === undefined)
                        activeGoongRequestId = seq
                }

                function applyGoongRoute(payload) {
                    if (!payload || !payload.routes || payload.routes.length === 0) {
                        rect_.routeError(qsTr("Không tìm thấy tuyến đường"))
                        return
                    }

                    const route = payload.routes[0]
                    const pts = pointsFromRoute(route)
                    if (pts.length < 2) {
                        rect_.routeError(qsTr("Không có hình học tuyến đường"))
                        return
                    }

                    fallbackLine.path = pts
                    root.usingFallbackRoute = true
                    root.hasRoute = true
                    var goongDistance = routeDistance(route)
                    if (!isFinite(goongDistance))
                        goongDistance = routePathDistance(pts)
                    root.activeRouteData = {
                        provider: "goong",
                        path: pts,
                        distance: goongDistance,
                        travelTime: routeDuration(route),
                        segments: routeSteps(route)
                    }

                    if (!rect_.contentRoute())
                        return
                    if (root.guidanceActive)
                        root.applyGuidanceCamera(false)
                    else
                        rect_.fitRoute()
                }
            }

            Connections {
                target: GoongRestClient
                function onReplyReady(requestId, api, payload) {
                    if (api !== "direction" || requestId !== httpRoute.activeGoongRequestId)
                        return
                    routeCtl.busy = false
                    httpRoute.applyGoongRoute(payload)
                }

                function onRequestFailed(requestId, api, message) {
                    if (api !== "direction" || requestId !== httpRoute.activeGoongRequestId)
                        return
                    routeCtl.busy = false
                    fallbackLine.path = []
                    root.usingFallbackRoute = false
                    root.hasRoute = false
                    rect_.routeError(message || qsTr("Dịch vụ Goong Direction không khả dụng"))
                }
            }

            //Routing

            QtObject {
                id: routeCtl
                property bool busy: false

                function requestRoute() {
                    busy = true;
                    root.activeRouteData = null
                    root.usingFallbackRoute = false
                    root.hasRoute = false
                    fallbackLine.path = []
                    rect_.clearRouteInfo(qsTr("Đang tính tuyến đường..."))

                    const start = root.startCoord
                    if (!root.hasStartPlace || !root.hasDestPlace ||
                            !start || !root.destCoord ||
                            !isFinite(start.latitude) || !isFinite(start.longitude) ||
                            !isFinite(root.destCoord.latitude) || !isFinite(root.destCoord.longitude)) {
                        busy = false
                        rect_.routeError(qsTr("Chọn điểm bắt đầu và điểm đến"))
                        return
                    }

                    console.log("Routing start=", start.latitude, start.longitude,
                                "dest=", root.destCoord.latitude, root.destCoord.longitude)

                    routeQuery.clearWaypoints()
                    routeQuery.addWaypoint(start)
                    routeQuery.addWaypoint(root.destCoord)
                    routeQuery.travelModes = RouteQuery.CarTravel
                    routeQuery.routeOptimizations = RouteQuery.ShortestRoute

                    if (GoongRestClient.available)
                        routeModel.reset()
                    else
                        routeModel.update()
                    httpRoute.drawCarRoute(start, root.destCoord)
                    mapView.map.center = start
                }

                //Component.onCompleted: requestRoute()
            }

            ListModel { id: suggestionModel }

            Item {
                id: searchOverlay
                x: 20; y: 20; z: 3
                width: 540
                height: searchPanel.height
                        + (quickChips.visible ? quickChips.height + 8 : 0)
                        + (suggestionPopup.visible ? suggestionPopup.height + 8 : 0)
                        + (routeSetupPanel.visible ? routeSetupPanel.height + 8 : 0)
                property bool guidanceAfterGeocode: false
                property bool hasSelectedPlace: false
                property bool suppressSearchUpdate: false
                property bool applyingTelex: false
                property bool searchStarted: false
                property bool enableVietnameseTelex: true
                property bool selectFirstWhenResultsArrive: false
                property bool dbusNavigationActive: false
                property int minSearchLength: 2
                property string mode: "destination"
                property real keyboardTop: keyboardPanel.open ? keyboardPanel.y : root.height
                property real maxSuggestionHeight: Math.min(420, Math.max(0, keyboardTop - searchOverlay.y - searchPanel.height - 16))
                property bool awaitingStartChoice: root.hasDestPlace && !root.hasStartPlace
                                                   && mode === "destination" && !dbusNavigationActive

                function searchBias() {
                    if (root.hasLivePos)
                        return root.gpsCoord
                    if (root.hasStartPlace)
                        return root.startCoord
                    return null
                }

                function requestSearch(query) {
                    const bias = searchBias()
                    SearchController.scheduleSearch(query,
                                                    bias ? bias.latitude : Number.NaN,
                                                    bias ? bias.longitude : Number.NaN)
                }

                function setSearchText(value) {
                    suppressSearchUpdate = true
                    nameSearch.text = value
                    nameSearch.cursorPosition = nameSearch.text.length
                    suppressSearchUpdate = false
                }

                function openKeyboard() {
                    keyboardPanel.open = true
                    if (!nameSearch.activeFocus)
                        nameSearch.forceActiveFocus()
                }

                function hideKeyboard() {
                    keyboardPanel.open = false
                    nameSearch.focus = false
                    Qt.inputMethod.hide()
                }

                function dismissSearchUi() {
                    hideKeyboard()
                    suggestionModel.clear()
                    SearchController.clear()
                    searchStarted = false
                    selectFirstWhenResultsArrive = false
                    guidanceAfterGeocode = false
                }

                function insertKeyboardText(value) {
                    var pos = Math.max(0, Math.min(nameSearch.cursorPosition, nameSearch.text.length))
                    var next = nameSearch.text.slice(0, pos) + value + nameSearch.text.slice(pos)
                    var nextPos = pos + value.length
                    if (enableVietnameseTelex && nextPos === next.length) {
                        const converted = applyVietnameseTelex(next)
                        if (converted !== next) {
                            next = converted
                            nextPos = converted.length
                        }
                    }
                    nameSearch.text = next
                    nameSearch.cursorPosition = nextPos
                }

                function backspaceKeyboardText() {
                    var pos = Math.max(0, Math.min(nameSearch.cursorPosition, nameSearch.text.length))
                    if (pos <= 0)
                        return
                    nameSearch.text = nameSearch.text.slice(0, pos - 1) + nameSearch.text.slice(pos)
                    nameSearch.cursorPosition = pos - 1
                }

                function applyTone(word, toneChar) {
                    const toneIndex = {"s": 0, "f": 1, "r": 2, "x": 3, "j": 4}[toneChar]
                    if (toneIndex === undefined)
                        return word

                    const toneMap = {
                        "a": ["á", "à", "ả", "ã", "ạ"],
                        "ă": ["ắ", "ằ", "ẳ", "ẵ", "ặ"],
                        "â": ["ấ", "ầ", "ẩ", "ẫ", "ậ"],
                        "e": ["é", "è", "ẻ", "ẽ", "ẹ"],
                        "ê": ["ế", "ề", "ể", "ễ", "ệ"],
                        "i": ["í", "ì", "ỉ", "ĩ", "ị"],
                        "o": ["ó", "ò", "ỏ", "õ", "ọ"],
                        "ô": ["ố", "ồ", "ổ", "ỗ", "ộ"],
                        "ơ": ["ớ", "ờ", "ở", "ỡ", "ợ"],
                        "u": ["ú", "ù", "ủ", "ũ", "ụ"],
                        "ư": ["ứ", "ừ", "ử", "ữ", "ự"],
                        "y": ["ý", "ỳ", "ỷ", "ỹ", "ỵ"]
                    }

                    for (var i = word.length - 1; i >= 0; --i) {
                        const ch = word.charAt(i)
                        const lower = ch.toLowerCase()
                        if (toneMap[lower]) {
                            const replacement = toneMap[lower][toneIndex]
                            return word.slice(0, i) + replacement + word.slice(i + 1)
                        }
                    }
                    return word
                }

                function applyTelexWord(word) {
                    if (word.length < 2)
                        return word

                    var converted = word
                    converted = converted.replace(/dd/g, "đ").replace(/DD/g, "Đ")
                    converted = converted.replace(/aa/g, "â").replace(/aw/g, "ă")
                    converted = converted.replace(/ee/g, "ê")
                    converted = converted.replace(/oo/g, "ô").replace(/ow/g, "ơ")
                    converted = converted.replace(/uw/g, "ư")

                    const last = converted.charAt(converted.length - 1).toLowerCase()
                    if ("sfrxj".indexOf(last) !== -1 && converted.length > 1) {
                        converted = applyTone(converted.slice(0, -1), last)
                    }
                    return converted
                }

                function applyVietnameseTelex(text) {
                    const parts = text.split(/(\s+)/)
                    for (var i = 0; i < parts.length; ++i) {
                        if (!/^\s+$/.test(parts[i]))
                            parts[i] = applyTelexWord(parts[i])
                    }
                    return parts.join("")
                }

                function handleSearchTextChanged() {
                    if (suppressSearchUpdate || applyingTelex)
                        return

                    hasSelectedPlace = false
                    var value = nameSearch.text
                    if (enableVietnameseTelex && nameSearch.cursorPosition === value.length) {
                        const converted = applyVietnameseTelex(value)
                        if (converted !== value) {
                            applyingTelex = true
                            nameSearch.text = converted
                            nameSearch.cursorPosition = converted.length
                            applyingTelex = false
                            value = converted
                        }
                    }

                    const query = value.trim()
                    if (query.length >= minSearchLength) {
                        searchStarted = true
                        requestSearch(query)
                    } else {
                        searchStarted = false
                        suggestionModel.clear()
                        SearchController.clear()
                    }
                }

                function showGuidance() {
                    root.guidanceActive = true
                    root.followPosition = true
                    mapView.followExternalCenter = true
                    display_route.y = root.height - display_route.height - 14
                    mapView.height = root.height
                    searchOverlay.visible = false
                    if (!root.hasRealGps && root.hasStartPlace && root.hasDestPlace)
                        root.currentHeading = root.guidanceBearing()
                    root.applyGuidanceCamera(true)
                    hideKeyboard()
                }

                function showRoutePreview() {
                    root.guidanceActive = false
                    root.followPosition = false
                    mapView.followExternalCenter = true
                    searchOverlay.visible = true
                    display_route.y = 800
                    mapView.height = root.height
                    if (root.hasStartPlace && root.hasDestPlace)
                        rect_.fitRoute()
                    else
                        root.applyPreviewCamera(true)
                    root.publishRouteState()
                    hideKeyboard()
                }

                function quickSearch(query) {
                    dbusNavigationActive = false
                    mode = "destination"
                    hasSelectedPlace = false
                    searchStarted = true
                    selectFirstWhenResultsArrive = false
                    setSearchText(query)
                    suggestionModel.clear()
                    SearchController.clear()
                    requestSearch(query)
                }

                function selectPlace(item, startGuidance) {
                    if (!item)
                        return

                    const label = item.display_name || item.primary || nameSearch.text
                    const coord = QtPositioning.coordinate(Number(item.lat), Number(item.lon))
                    var useVehicleStart = false
                    if (mode === "start") {
                        root.startCoord = coord
                        root.startName = label
                        root.hasStartPlace = true
                        mode = "destination"
                        setSearchText(root.destName || label)
                    } else {
                        root.destCoord = coord
                        root.destName = label
                        root.hasDestPlace = true
                        root.hasRoute = false
                        setSearchText(label)
                        useVehicleStart = dbusNavigationActive && !root.hasStartPlace
                    }

                    hasSelectedPlace = true
                    searchStarted = false
                    suggestionModel.clear()
                    SearchController.clear()
                    hideKeyboard()

                    if (useVehicleStart) {
                        if (root.hasLivePos) {
                            useGpsStart(true)
                        } else {
                            rect_.routeError(qsTr("Đang chờ tín hiệu định vị của xe"))
                        }
                        return
                    }

                    if (root.hasStartPlace && root.hasDestPlace) {
                        routeCtl.requestRoute()
                        if (startGuidance)
                            showGuidance()
                        dbusNavigationActive = false
                    }
                }

                function navigateFromVehiclePosition(destination) {
                    const query = (destination || "").trim()
                    if (query.length === 0)
                        return

                    dbusNavigationActive = true
                    mode = "destination"
                    guidanceAfterGeocode = false
                    selectFirstWhenResultsArrive = false
                    hasSelectedPlace = false
                    searchStarted = true

                    root.hasDestPlace = false
                    root.destName = ""
                    root.activeRouteData = null
                    root.hasRoute = false
                    fallbackLine.path = []
                    root.usingFallbackRoute = false

                    if (root.hasLivePos) {
                        root.startCoord = root.gpsCoord
                        root.startName = root.currentLocationName()
                        root.hasStartPlace = true
                    } else {
                        root.hasStartPlace = false
                        root.startName = ""
                        rect_.routeError(qsTr("Đang chờ tín hiệu định vị của xe"))
                    }

                    searchOverlay.visible = true
                    display_route.y = 800
                    mapView.height = root.height
                    root.guidanceActive = false
                    mapView.followExternalCenter = true
                    root.applyPreviewCamera(true)
                    setSearchText(query)
                    suggestionModel.clear()
                    SearchController.clear()
                    requestSearch(query)
                    openKeyboard()
                }

                function useGpsStart(startGuidance) {
                    if (!root.hasLivePos) {
                        rect_.routeError(qsTr("Chưa có tín hiệu định vị. Hãy nhập điểm bắt đầu"))
                        return
                    }

                    root.startCoord = root.gpsCoord
                    root.startName = root.currentLocationName()
                    root.hasStartPlace = true
                    mapView.followExternalCenter = true
                    suggestionModel.clear()
                    SearchController.clear()
                    hideKeyboard()

                    if (root.hasDestPlace) {
                        routeCtl.requestRoute()
                        if (startGuidance)
                            showGuidance()
                        dbusNavigationActive = false
                    }
                }

                function inputStartPoint() {
                    dbusNavigationActive = false
                    mode = "start"
                    hasSelectedPlace = false
                    searchStarted = false
                    selectFirstWhenResultsArrive = false
                    setSearchText("")
                    suggestionModel.clear()
                    SearchController.clear()
                    openKeyboard()
                }

                function resetCurrentSearch() {
                    dbusNavigationActive = false
                    setSearchText("")
                    hasSelectedPlace = false
                    searchStarted = false
                    selectFirstWhenResultsArrive = false
                    suggestionModel.clear()
                    SearchController.clear()

                    if (mode === "destination") {
                        root.hasDestPlace = false
                        root.destName = ""
                        root.activeRouteData = null
                        root.startCoord = root.gpsCoord
                        root.startName = root.currentLocationName()
                        root.hasStartPlace = true
                        fallbackLine.path = []
                        root.usingFallbackRoute = false
                        root.hasRoute = false
                        root.guidanceActive = false
                        mapView.followExternalCenter = true
                    } else {
                        root.hasStartPlace = false
                        root.startName = ""
                        root.activeRouteData = null
                        root.hasRoute = false
                        root.guidanceActive = false
                    }

                    display_route.y = 800
                    mapView.height = root.height
                    mapView.followExternalCenter = true
                    root.applyPreviewCamera(true)
                    root.publishRouteState()
                    openKeyboard()
                }

                function submit(startGuidance) {
                    const query = nameSearch.text.trim()
                    if (startGuidance && hasSelectedPlace && root.hasStartPlace && root.hasDestPlace) {
                        routeCtl.requestRoute()
                        showGuidance()
                        return
                    }

                    if (suggestionModel.count > 0) {
                        selectPlace(suggestionModel.get(0), startGuidance)
                        return
                    }

                    if (query.length >= minSearchLength) {
                        if (SearchController.searching) {
                            guidanceAfterGeocode = startGuidance
                            selectFirstWhenResultsArrive = true
                            return
                        }

                        guidanceAfterGeocode = startGuidance
                        selectFirstWhenResultsArrive = true
                        searchStarted = true
                        requestSearch(query)
                    }
                }

                Rectangle {
                    id: searchPanel
                    width: parent.width
                    height: 56
                    radius: 10
                    color: "#ffffff"
                    border.width: 1
                    border.color: "#e5e7eb"
                    layer.enabled: true
                    layer.effect: DropShadow {
                        horizontalOffset: 0
                        verticalOffset: 3
                        radius: 14
                        samples: 28
                        color: "#45000000"
                    }

                    Image {
                        id: searchGlyph
                        x: 16
                        width: 24; height: 24
                        anchors.verticalCenter: parent.verticalCenter
                        visible: false
                    }

                    MapIcon {
                        x: 16
                        width: 24; height: 24
                        anchors.verticalCenter: parent.verticalCenter
                        iconName: "search"
                        iconColor: "#5f6368"
                    }

                    Control.TextField {
                        id: nameSearch
                        x: 52
                        width: parent.width - 132
                        height: parent.height
                        anchors.verticalCenter: parent.verticalCenter
                        placeholderText: searchOverlay.mode === "start" ? qsTr("Tìm điểm bắt đầu") : qsTr("Tìm điểm đến")
                        font.family: "Arial"
                        font.pixelSize: 17
                        color: "#202124"
                        selectByMouse: true
                        readOnly: true
                        cursorVisible: activeFocus
                        inputMethodHints: Qt.ImhPreferLowercase
                        background: Item {}
                        onAccepted: searchOverlay.submit(false)
                        onTextChanged: searchOverlay.handleSearchTextChanged()
                        onActiveFocusChanged: {
                            if (activeFocus)
                                searchOverlay.openKeyboard()
                        }

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            onClicked: {
                                nameSearch.cursorPosition = nameSearch.text.length
                                searchOverlay.openKeyboard()
                            }
                        }
                    }

                    Control.Button {
                        id: clearSearch
                        x: parent.width - 74
                        width: 30; height: 30
                        anchors.verticalCenter: parent.verticalCenter
                        visible: nameSearch.text.length > 0
                        focusPolicy: Qt.NoFocus
                        background: Rectangle {
                            radius: 15
                            color: clearSearch.pressed ? "#e8eaed" : (clearSearch.hovered ? "#f1f3f4" : "transparent")
                        }
                        contentItem: Text {
                            text: "×"
                            color: "#5f6368"
                            font.pixelSize: 16
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            searchOverlay.resetCurrentSearch()
                        }
                    }

                    MapRoundButton {
                        id: search
                        x: parent.width - 42
                        width: 34; height: 34
                        anchors.verticalCenter: parent.verticalCenter
                        iconName: "search"
                        iconColor: "#1a73e8"
                        buttonColor: "#f1f3f4"
                        pressedColor: "#dbeafe"
                        onPressed: searchOverlay.submit(false)
                    }
                }

                Row {
                    id: quickChips
                    y: searchPanel.height + 8
                    height: 38
                    spacing: 8
                    visible: searchOverlay.visible
                             && searchOverlay.mode === "destination"
                             && nameSearch.text.length === 0
                             && !root.hasRoute

                    Repeater {
                        model: [qsTr("Nhà hàng"), qsTr("Cà phê"), qsTr("Trạm xăng"), qsTr("Bãi đỗ xe")]

                        Control.Button {
                            id: quickChipButton
                            height: 38
                            width: Math.max(92, quickChipText.implicitWidth + 28)
                            text: modelData
                            focusPolicy: Qt.NoFocus
                            onClicked: searchOverlay.quickSearch(modelData)
                            background: Rectangle {
                                radius: 19
                                color: quickChipButton.pressed ? "#dbeafe" : (quickChipButton.hovered ? "#eff6ff" : "#ffffff")
                                border.width: 1
                                border.color: "#dadce0"
                            }
                            contentItem: Text {
                                id: quickChipText
                                text: quickChipButton.text
                                color: "#1a73e8"
                                font.pixelSize: 13
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                Rectangle {
                    id: routeSetupPanel
                    y: searchPanel.height + (quickChips.visible ? quickChips.height + 8 : 0) + 8
                    width: searchPanel.width
                    height: 168
                    visible: searchOverlay.visible && searchOverlay.awaitingStartChoice
                             && !suggestionPopup.visible && !nameSearch.activeFocus
                    radius: 10
                    color: "#ffffff"
                    border.width: 1
                    border.color: "#e5e7eb"
                    layer.enabled: true
                    layer.effect: DropShadow {
                        horizontalOffset: 0
                        verticalOffset: 3
                        radius: 14
                        samples: 28
                        color: "#42000000"
                    }

                    Text {
                        x: 20; y: 14
                        width: parent.width - 40
                        height: 22
                        text: qsTr("Điểm đến") + ": " + root.destName
                        color: "#202124"
                        font.pixelSize: 15
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        x: 20; y: 42
                        width: parent.width - 40
                        height: 22
                        text: qsTr("Chọn điểm bắt đầu")
                        color: "#5f6368"
                        font.pixelSize: 13
                        elide: Text.ElideRight
                    }

                    Control.Button {
                        id: gpsStartButton
                        x: 20; y: 76
                        width: parent.width / 2 - 28
                        height: 48
                        enabled: root.hasLivePos
                        text: root.hasRealGps ? qsTr("Dùng định vị hiện tại") : qsTr("Dùng vị trí mẫu")
                        onClicked: searchOverlay.useGpsStart(false)
                        background: Rectangle {
                            radius: 8
                            color: gpsStartButton.enabled
                                   ? (gpsStartButton.pressed ? "#dbeafe" : (gpsStartButton.hovered ? "#eff6ff" : "#e8f0fe"))
                                   : "#f1f3f4"
                            border.width: 1
                            border.color: gpsStartButton.enabled ? "#bfdbfe" : "#e5e7eb"
                        }
                        contentItem: Text {
                            text: gpsStartButton.text
                            color: gpsStartButton.enabled ? "#174ea6" : "#9aa0a6"
                            font.pixelSize: 13
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }

                    Control.Button {
                        id: manualStartButton
                        x: parent.width / 2 + 8; y: 76
                        width: parent.width / 2 - 28
                        height: 48
                        text: qsTr("Nhập điểm bắt đầu")
                        onClicked: searchOverlay.inputStartPoint()
                        background: Rectangle {
                            radius: 8
                            color: manualStartButton.pressed ? "#e8eaed" : (manualStartButton.hovered ? "#f8fafd" : "#ffffff")
                            border.width: 1
                            border.color: "#dadce0"
                        }
                        contentItem: Text {
                            text: manualStartButton.text
                            color: "#202124"
                            font.pixelSize: 13
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }

                    Text {
                        x: 20; y: 132
                        width: parent.width - 40
                        height: 20
                        text: root.hasRealGps ? qsTr("Định vị đã sẵn sàng") : qsTr("Đang dùng vị trí mẫu: 125/16/11 Bùi Đình Túy")
                        color: root.hasLivePos ? "#1a73e8" : "#b3261e"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }

                Rectangle {
                    id: suggestionPopup
                    y: searchPanel.height + (quickChips.visible ? quickChips.height + 8 : 0) + 8
                    width: searchPanel.width
                    property real fixedRowsHeight: (SearchController.searching && suggestionModel.count === 0 ? 48 : 0)
                                                   + (SearchController.statusMessage.length > 0 ? 54 : 0)
                                                   + (searchOverlay.searchStarted && !SearchController.searching
                                                      && SearchController.statusMessage.length === 0 && suggestionModel.count === 0 ? 54 : 0)
                    height: Math.min(searchOverlay.maxSuggestionHeight, fixedRowsHeight + suggestionView.height)
                    visible: searchOverlay.visible && nameSearch.text.trim().length >= searchOverlay.minSearchLength
                             && searchOverlay.maxSuggestionHeight >= 48
                             && (suggestionModel.count > 0 || SearchController.searching
                                 || SearchController.statusMessage.length > 0 || searchOverlay.searchStarted)
                    radius: 10
                    color: "#ffffff"
                    clip: true
                    layer.enabled: true
                    layer.effect: DropShadow {
                        horizontalOffset: 0
                        verticalOffset: 3
                        radius: 14
                        samples: 28
                        color: "#42000000"
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        preventStealing: true
                        onPressed: function(mouse) {
                            mouse.accepted = true
                        }
                    }

                    Column {
                        id: suggestionContent
                        width: parent.width
                        height: childrenRect.height

                        ItemDelegate {
                            width: parent.width
                            height: 48
                            visible: SearchController.searching && suggestionModel.count === 0
                            enabled: false
                            contentItem: Item {
                                Text {
                                    x: 52
                                    width: parent.width - 68
                                    height: parent.height
                                    text: qsTr("Đang tìm kiếm...")
                                    color: "#5f6368"
                                    font.pixelSize: 14
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                            }
                        }

                        ItemDelegate {
                            width: parent.width
                            height: 54
                            visible: SearchController.statusMessage.length > 0
                            enabled: false
                            contentItem: Item {
                                Text {
                                    x: 52
                                    width: parent.width - 68
                                    height: parent.height
                                    text: SearchController.statusMessage
                                    color: "#b3261e"
                                    font.pixelSize: 13
                                    verticalAlignment: Text.AlignVCenter
                                    wrapMode: Text.Wrap
                                    maximumLineCount: 2
                                    elide: Text.ElideRight
                                }
                            }
                        }

                        ItemDelegate {
                            width: parent.width
                            height: 54
                            visible: searchOverlay.searchStarted && !SearchController.searching
                                     && SearchController.statusMessage.length === 0 && suggestionModel.count === 0
                            enabled: false
                            contentItem: Item {
                                Text {
                                    x: 52
                                    width: parent.width - 68
                                    height: parent.height
                                    text: qsTr("Không tìm thấy gợi ý")
                                    color: "#5f6368"
                                    font.pixelSize: 14
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                            }
                        }

                        ListView {
                            id: suggestionView
                            model: suggestionModel
                            width: parent.width
                            height: Math.max(0, Math.min(contentHeight, searchOverlay.maxSuggestionHeight - suggestionPopup.fixedRowsHeight))
                            interactive: contentHeight > height
                            pressDelay: 170
                            clip: true
                            boundsBehavior: Flickable.StopAtBounds
                            flickableDirection: Flickable.VerticalFlick
                            maximumFlickVelocity: 4600
                            flickDeceleration: 3000
                            ScrollBar.vertical: ScrollBar { }
                            onMovementStarted: {
                                searchOverlay.hideKeyboard()
                                root.manualMapGestureActive = true
                            }
                            onMovementEnded: root.manualMapGestureActive = false
                            onFlickEnded: root.manualMapGestureActive = false

                            delegate: ItemDelegate {
                                width: suggestionView.width
                                height: 96
                                hoverEnabled: true
                                text: primary
                                background: Rectangle {
                                    color: parent.pressed ? "#e8f0fe" : (parent.hovered ? "#f8fafd" : "#ffffff")
                                }
                                contentItem: Item {
                                    anchors.fill: parent

                                    Rectangle {
                                        x: 16
                                        width: 32; height: 32
                                        radius: 16
                                        anchors.verticalCenter: parent.verticalCenter
                                        color: "#e8f0fe"
                                        Rectangle {
                                            width: 10; height: 10
                                            radius: 5
                                            anchors.centerIn: parent
                                            color: "#1a73e8"
                                        }
                                    }

                                    Column {
                                        x: 60
                                        width: parent.width - 88
                                        anchors.verticalCenter: parent.verticalCenter
                                        spacing: 3

                                        Text {
                                            text: primary
                                            color: "#202124"
                                            font.pixelSize: 15
                                            font.bold: true
                                            wrapMode: Text.Wrap
                                            maximumLineCount: 2
                                            elide: Text.ElideRight
                                            width: parent.width
                                        }
                                        Text {
                                            text: (typeLabel || qsTr("Địa điểm")) + (distanceText ? " - " + distanceText : "")
                                            color: "#5f6368"
                                            font.pixelSize: 12
                                            elide: Text.ElideRight
                                            width: parent.width
                                        }
                                        Text {
                                            text: secondary
                                            color: "#5f6368"
                                            font.pixelSize: 12
                                            wrapMode: Text.Wrap
                                            maximumLineCount: 2
                                            elide: Text.ElideRight
                                            width: parent.width
                                            visible: text.length > 0
                                        }
                                    }
                                }
                                onClicked: searchOverlay.selectPlace({
                                    "lat": lat,
                                    "lon": lon,
                                    "primary": primary,
                                    "secondary": secondary,
                                    "display_name": display_name
                                }, false)
                            }
                        }
                    }
                }
            }

            Item {
                id: keyboardDismissLayer
                z: 2.5
                x: 0
                y: 0
                width: root.width
                height: keyboardPanel.open ? keyboardPanel.y : root.height
                visible: keyboardPanel.open || suggestionPopup.visible

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    onPressed: searchOverlay.dismissSearchUi()
                }
            }

            Rectangle {
                id: routePreview
                z: 2
                x: 20
                y: root.height - height - 18
                width: root.width - 40
                height: routeInfoModel.count > 0 ? 188 : 128
                radius: 16
                color: "#ffffff"
                visible: searchOverlay.visible
                         && searchOverlay.mode === "destination"
                         && root.hasRoute
                         && root.hasDestPlace
                         && !keyboardPanel.open
                         && !suggestionPopup.visible
                         && !routeSetupPanel.visible
                layer.enabled: true
                layer.effect: DropShadow {
                    horizontalOffset: 0
                    verticalOffset: 4
                    radius: 18
                    samples: 32
                    color: "#45000000"
                }

                Column {
                    x: 20
                    y: 16
                    width: parent.width - 210
                    spacing: 5

                    Text {
                        width: parent.width
                        text: root.destName
                        color: "#202124"
                        font.pixelSize: 18
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: (time.contentText || qsTr("--")) + " · " + (distance.contentText || qsTr("--"))
                        color: "#188038"
                        font.pixelSize: 15
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        width: parent.width
                        text: root.startName.length > 0
                              ? qsTr("Từ ") + root.startName
                              : qsTr("Chưa chọn điểm bắt đầu")
                        color: "#5f6368"
                        font.pixelSize: 13
                        elide: Text.ElideRight
                    }
                }

                ListView {
                    id: routePreviewSteps
                    x: 20
                    y: 96
                    width: parent.width - 210
                    height: parent.height - y - 12
                    visible: routeInfoModel.count > 0
                    model: routeInfoModel
                    interactive: contentHeight > height
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    delegate: Item {
                        width: routePreviewSteps.width
                        height: 30

                        Image {
                            x: 0
                            y: 5
                            width: 18
                            height: 18
                            source: icon || "images/icons8-arrow-up-94.png"
                            fillMode: Image.PreserveAspectFit
                        }

                        Text {
                            x: 28
                            y: 0
                            width: parent.width - 106
                            height: parent.height
                            text: instruction
                            color: "#3c4043"
                            font.pixelSize: 12
                            font.bold: index === 0
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }

                        Text {
                            x: parent.width - 72
                            y: 0
                            width: 72
                            height: parent.height
                            text: distance
                            color: "#5f6368"
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }
                }

                Control.Button {
                    id: startGuidanceButton
                    x: parent.width - 176
                    y: 18
                    width: 136
                    height: 48
                    text: qsTr("Bắt đầu")
                    onClicked: searchOverlay.showGuidance()
                    background: Rectangle {
                        radius: 24
                        color: startGuidanceButton.pressed ? "#1558d6" : (startGuidanceButton.hovered ? "#1967d2" : "#1a73e8")
                    }
                    contentItem: Text {
                        text: startGuidanceButton.text
                        color: "white"
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Row {
                    x: parent.width - 176
                    y: 76
                    width: 136
                    height: 34
                    spacing: 8

                    Control.Button {
                        id: editStartButton
                        width: 64
                        height: 34
                        text: qsTr("Sửa")
                        onClicked: searchOverlay.inputStartPoint()
                        background: Rectangle {
                            radius: 17
                            color: editStartButton.pressed ? "#e8eaed" : (editStartButton.hovered ? "#f8fafd" : "#ffffff")
                            border.width: 1
                            border.color: "#dadce0"
                        }
                        contentItem: Text {
                            text: editStartButton.text
                            color: "#1a73e8"
                            font.pixelSize: 13
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Control.Button {
                        id: clearRouteButton
                        width: 64
                        height: 34
                        text: qsTr("Xóa")
                        onClicked: searchOverlay.resetCurrentSearch()
                        background: Rectangle {
                            radius: 17
                            color: clearRouteButton.pressed ? "#e8eaed" : (clearRouteButton.hovered ? "#f8fafd" : "#ffffff")
                            border.width: 1
                            border.color: "#dadce0"
                        }
                        contentItem: Text {
                            text: clearRouteButton.text
                            color: "#5f6368"
                            font.pixelSize: 13
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            MapKeyboard {
                id: keyboardPanel
                z: 10
                x: 12
                width: root.width - 24
                property bool open: false
                y: open ? root.height - height - 8 : root.height + 12
                visible: open || y < root.height
                onInsertText: function(text) { searchOverlay.insertKeyboardText(text) }
                onBackspacePressed: searchOverlay.backspaceKeyboardText()
                onSearchPressed: searchOverlay.submit(false)
                onHidePressed: searchOverlay.hideKeyboard()
                Behavior on y { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
            }

            MapRoundButton {
                id: map3dToggle
                x: root.width - 68; y: routePreview.visible ? 260 : (searchOverlay.visible ? 410 : 200)
                width: 46; height: 46
                iconName: root.map3dEnabled ? "2d" : "3d"
                iconColor: root.map3dEnabled ? "#1a73e8" : "#3c4043"
                buttonColor: root.map3dEnabled ? "#e8f0fe" : "#ffffff"
                pressedColor: "#dbeafe"
                onClicked: root.toggleMap3d()
            }

            MapRoundButton {
                id: myLocation
                x: root.width - 68; y: routePreview.visible ? 310 : (searchOverlay.visible ? 460 : 250)
                width: 46; height: 46
                iconName: "location"
                iconColor: root.followPosition ? "#1a73e8" : "#3c4043"
                buttonColor: "#ffffff"
                pressedColor: "#e8f0fe"
                onPressed: {
                    root.followPosition = true
                    mapView.followExternalCenter = true
                    if (root.guidanceActive) {
                        root.applyGuidanceCamera(true)
                    } else {
                        mapView.map.center = root.currentCoord
                        mapView.map.zoomLevel = Math.max(mapView.map.zoomLevel, 15)
                        root.applyPreviewCamera(false)
                    }
                }
            }

            MapRoundButton {
                id: zoomPlus
                x: root.width - 68; y: routePreview.visible ? 360 : (searchOverlay.visible ? 510 : 300)
                width: 46; height: 46
                iconName: "plus"
                iconColor: "#3c4043"
                buttonColor: "#ffffff"
                pressedColor: "#e8f0fe"
                onClicked: {
                    root.followPosition = false
                    mapView.followExternalCenter = false
                    var maxZoom = isFinite(mapView.map.maximumZoomLevel) ? mapView.map.maximumZoomLevel : 20
                    mapView.map.zoomLevel = Math.min(maxZoom, mapView.map.zoomLevel + 1)
                }
            }

            MapRoundButton {
                id: zoomSubtract
                x: root.width - 68; y: routePreview.visible ? 410 : (searchOverlay.visible ? 560 : 350)
                width: 46; height: 46
                iconName: "minus"
                iconColor: "#3c4043"
                buttonColor: "#ffffff"
                pressedColor: "#e8f0fe"
                onClicked: {
                    root.followPosition = false
                    mapView.followExternalCenter = false
                    var minZoom = isFinite(mapView.map.minimumZoomLevel) ? mapView.map.minimumZoomLevel : 2
                    mapView.map.zoomLevel = Math.max(minZoom, mapView.map.zoomLevel - 1)
                }
            }

            Rectangle {
                id: display_route
                x: 14; y: 700; width: mapView.width - 28; height: 248; color: "#ffffff"; radius: 24
                border.width: 0
                layer.enabled: true
                layer.effect: DropShadow {
                    horizontalOffset: 0
                    verticalOffset: -4
                    radius: 28
                    samples: 40
                    color: "#38000000"
                }
                Behavior on y{NumberAnimation{duration: 200; easing.type: Easing.InOutBack}}

                Rectangle {
                    id: current_guide
                    x: 14; y: 12; width: parent.width - 28; height: 98; color: "#188038"; radius: 24
                    border.width: 0

                    Rectangle {
                        x: 18; y: 19; width: 68; height: 68
                        radius: 34
                        color: "#ffffff"
                    }

                    Image {
                        id: image_turn
                        x: 28; y: 29; width: 48; height: 48
                        source: "images/icons8-redo-94.png"; fillMode: Image.PreserveAspectFit
                    }

                    TextEffect {
                        id: ins
                        x: 104; y: 12; width: parent.width - 200; height: 52; contentText: qsTr(""); fontSize: 22; wrapMode_: Text.Wrap; color: "#ffffff"; shadow: "transparent"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    TextEffect {
                        id: distance_target
                        x: 104; y: 68; width: parent.width - 200; height: 24; contentText: qsTr(""); fontSize: 18; color: "#e6f4ea"; shadow: "transparent"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Rectangle {
                    id: time_distance
                    x: 20; y: 118; width: 306; height: 46
                    radius: 18
                    color: "#eef2f6"
                    border.width: 1
                    border.color: "#dde4ec"

                    TextEffect {
                        id: time
                        x: 16; y: 1; width: 132; height: 27; contentText: qsTr(""); fontSize: 19; color: "#188038"; shadow: "transparent"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    Rectangle {
                        x: 152; y: 9; width: 1; height: 28
                        color: "#d3dbe5"
                    }

                    TextEffect {
                        id: distance
                        x: 166; y: 1; width: parent.width - 182; height: 27; contentText: qsTr(""); fontSize: 18; color: "#188038"; shadow: "transparent"
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        x: 16; y: 28; width: 58; height: 16
                        text: qsTr("Đến")
                        color: "#5f6368"
                        font.pixelSize: 12
                        font.bold: true
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    Timer {
                        id: timer
                        interval: 1000
                        running: root.hasRoute
                        repeat: true
                        onTriggered: rect_.updateEstimate()
                    }
                    TextEffect {
                        id: time_estimate
                        x: 70; y: 25; width: 78; height: 20; contentText: qsTr(""); fontSize: 14; color: "#5f6368"; shadow: "transparent"
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                    }

                    Text {
                        x: 166; y: 28; width: parent.width - 182; height: 16
                        text: qsTr("Quãng đường")
                        color: "#5f6368"
                        font.pixelSize: 12
                        font.bold: true
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }

                Rectangle {
                    id: after_guide
                    x: 340; y: 118; width: parent.width - 360; height: 46; color: "#eef2f6"; radius: 18
                    border.width: 1
                    border.color: "#dde4ec"

                    Text {
                        x: 16; y: 0; width: 36; height: parent.height
                        text: qsTr("Sau")
                        color: "#5f6368"
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    Image {
                        x: 58; y: 12; width: 22; height: 22
                        source: root.nextGuideIcon
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        x: 92; y: 0; width: parent.width - 108; height: parent.height
                        text: root.nextGuideText.length > 0 ? root.nextGuideText : ins.contentText
                        color: "#3c4043"
                        font.pixelSize: 13
                        font.bold: true
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }

                ListView {
                    id: guidanceSteps
                    x: 20
                    y: 176
                    width: parent.width - 40
                    height: 56
                    model: routeInfoModel
                    visible: routeInfoModel.count > 0
                    interactive: contentHeight > height
                    clip: true
                    boundsBehavior: Flickable.StopAtBounds

                    delegate: Rectangle {
                        width: guidanceSteps.width
                        height: 28
                        color: index === 0 ? "#e6f4ea" : "transparent"
                        radius: 14

                        Image {
                            x: 10
                            y: 5
                            width: 18
                            height: 18
                            source: icon || "images/icons8-arrow-up-94.png"
                            fillMode: Image.PreserveAspectFit
                        }

                        Text {
                            x: 38
                            y: 0
                            width: parent.width - 128
                            height: parent.height
                            text: instruction
                            color: index === 0 ? "#188038" : "#3c4043"
                            font.pixelSize: 12
                            font.bold: index === 0
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }

                        Text {
                            x: parent.width - 82
                            y: 0
                            width: 72
                            height: parent.height
                            text: distance
                            color: "#5f6368"
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }
                }

                Control.Button {
                    id: quit
                    x: parent.width - 82; y: 39; width: 52; height: 52
                    text: qsTr("Thoát")
                    focusPolicy: Qt.NoFocus
                    onClicked: {
                        searchOverlay.showRoutePreview()
                    }
                    background: Rectangle {
                        radius: 26
                        color: quit.pressed ? "#e6f4ea" : "#ffffff"
                        border.width: 0
                    }
                    contentItem: Text {
                        text: "×"
                        color: "#174ea6"
                        font.pixelSize: 30
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    Connections {
        target: SearchController
        function onResultsChanged() {
            suggestionModel.clear()
            var list = SearchController.results
            for (var i = 0; i < list.length; ++i)
                suggestionModel.append(list[i])

            if (searchOverlay.selectFirstWhenResultsArrive && !searchOverlay.dbusNavigationActive) {
                searchOverlay.selectFirstWhenResultsArrive = false
                if (suggestionModel.count > 0)
                    searchOverlay.selectPlace(suggestionModel.get(0), searchOverlay.guidanceAfterGeocode)
                else
                    searchOverlay.guidanceAfterGeocode = false
            }
        }

        function onStatusMessageChanged() {
            if (SearchController.statusMessage.length > 0) {
                searchOverlay.selectFirstWhenResultsArrive = false
                searchOverlay.guidanceAfterGeocode = false
            }
        }
    }

    Connections {
        target: NavigationDBusService
        function onDestinationRequested(destination) {
            searchOverlay.navigateFromVehiclePosition(destination)
        }
    }

}
