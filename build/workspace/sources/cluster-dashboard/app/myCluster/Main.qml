import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QtPositioning
import Fuel 1.0
import "./QML"


Window{
    id: window
    width: 1280
    height: 800
    visible: true
    visibility: Window.Windowed
    flags: Qt.Window | Qt.FramelessWindowHint
    title: qsTr("DashBoard")

    property bool clusterReadyToShow: false

    StartupSplash {
        anchors.fill: parent
        z: 10
        opacity: window.clusterReadyToShow ? 0 : 1
        visible: opacity  > 0

        Behavior on opacity {
            NumberAnimation { duration: 180 }
        }
    }

    Loader {
        id: shellContent
        anchors.fill: parent
        active: false
        visible: true
        opacity: window.clusterReadyToShow ? 1 : 0
        sourceComponent: shellComponent

        Behavior on opacity {
            NumberAnimation { duration: 180 }
        }
    }

    Timer {
        interval: 150
        running: true
        repeat: false
        onTriggered: shellContent.active = true
    }

    Component {
        id: shellComponent

        Item{
            id: root
            anchors.fill: parent

            FontLoader {
                id: dashboardFont
                source: "qrc:/asset/fonts/Roboto-BoldCondensed.ttf"
            }

            readonly property real baseWidth: 1280
            readonly property real baseHeight: 800
            readonly property real uiScale: Math.min(width / baseWidth, height / baseHeight)
            readonly property string textFontFamily: dashboardFont.status === FontLoader.Ready && dashboardFont.name.length > 0
                                                        ? dashboardFont.name
                                                        : (typeof dashboardTextFontFamily === "undefined" ? "Sans Serif" : dashboardTextFontFamily)
            readonly property int topBarWidth: s(620)
            readonly property int topBarHeight: s(115)
            readonly property int bottomBarWidth: s(650)
            readonly property int bottomBarHeight: s(92)
            readonly property int warningButtonSize: s(40)
            readonly property int warningIconSize: s(34)
            readonly property int gaugeSize: s(455)
            readonly property int gaugeSideMargin: s(26)
            readonly property int gaugeYOffset: s(42)
            readonly property int fuelArcWidth: s(224)
            readonly property int fuelArcHeight: s(202)
            readonly property int modelParkWidth: s(560)
            readonly property int modelParkHeight: s(500)
            readonly property int modelDriveWidth: s(520)
            readonly property int modelDriveHeight: s(430)
            readonly property int gearButtonSize: s(48)
            readonly property int gearSpacing: s(50)
            readonly property int gearOuterGap: s(20)
            readonly property int cameraTransitionDuration: 350
            readonly property int load3DDelayMs: 250
            readonly property bool have3DAssets: typeof external3DAssetsAvailable === "undefined" ? true : external3DAssetsAvailable
            readonly property bool allow3DModel: have3DAssets && (typeof external3DEnabled === "undefined" ? true : external3DEnabled)
            property bool demoMode: false
            property var serialTraceValues: ({ "speed": -1, "fuel": -1, "coolant": -1 })
            readonly property var mapBuiDinhTuyCoordinate: QtPositioning.coordinate(10.80690, 106.70290)
            readonly property var mapLandmark81Coordinate: QtPositioning.coordinate(10.79504, 106.72186)
            readonly property var mapHcmuteCoordinate: QtPositioning.coordinate(10.85056, 106.77193)
            readonly property var mapCenterCoordinate: mapBuiDinhTuyCoordinate
            property var mapDriveStartCoordinate: mapBuiDinhTuyCoordinate
            property var mapDriveEndCoordinate: mapHcmuteCoordinate
            property string mapPhoneDestinationName: ""
            property var mapRoutePath: []
            property var mapRouteGuidanceSteps: []
            property int mapRouteRequestId: -1
            property int mapRouteRequestEpoch: -1
            property int mapRouteEpoch: 0
            property int mapNavigationSessionId: 0
            property bool mapRouteReady: false
            property bool mapRouteProgressReady: false
            property real mapDriveSpeedKph: 0
            property bool mapArrivalHandled: false
            property bool mapArrivalPanelVisible: false
            property bool enable3DAnimation: true
            property bool useHighQuality3D: true
            property bool load3DModel: false
            property bool startupModelSettled: false
            property bool startupSelfCheckDone: false
            property int startupPhase: 0
            property real startupBlink: 0
            property real startupSweep: 0
            property bool startupStaticReadySignalSent: false
            property bool startupReadySignalSent: false
            property bool startupSelfCheckStarted: false
            property int pendingCarKeyFrame: 360
            property bool pendingRearCamera: false
            readonly property bool startupSelfCheckActive: startupPhase === 1
            readonly property bool startupStaticComponentsReady: dashboardFont.status === FontLoader.Ready
                                                                && backgroundPanel.status === Image.Ready
                                                                && panel.status === Image.Ready
                                                                && topBar.status === Image.Ready
                                                                && leftgauge.status === Image.Ready
                                                                && rightgaugae.status === Image.Ready
                                                                && bottomGearBar.status === Image.Ready
            readonly property bool startupModelReady: !allow3DModel || startupModelSettled
            readonly property bool startupComponentsReady: startupStaticComponentsReady && startupModelReady
            readonly property string mapArrivalText: mapPhoneDestinationName.length > 0
                                    ? qsTr("Đã đến ") + mapPhoneDestinationName
                                    : qsTr("Đã đến điểm đến")

            function coordValid(coord) {
                return coord
                        && isFinite(coord.latitude) && isFinite(coord.longitude)
                        && coord.latitude >= -90 && coord.latitude <= 90
                        && coord.longitude >= -180 && coord.longitude <= 180
            }

            function appendRoutePoint(points, point) {
                if (!coordValid(point))
                    return
                if (points.length > 0) {
                    var last = points[points.length - 1]
                    if (Math.abs(last.latitude - point.latitude) < 0.000001
                            && Math.abs(last.longitude - point.longitude) < 0.000001)
                        return
                }
                points.push(point)
            }

            function decodeGoongPolyline(encoded) {
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
                    lat += (result & 1) ? ~(result >> 1) : (result >> 1)

                    shift = 0
                    result = 0
                    do {
                        b = encoded.charCodeAt(index++) - 63
                        result |= (b & 0x1f) << shift
                        shift += 5
                    } while (b >= 0x20 && index < encoded.length)
                    lng += (result & 1) ? ~(result >> 1) : (result >> 1)

                    appendRoutePoint(points, QtPositioning.coordinate(lat / 100000.0, lng / 100000.0))
                }

                return points
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

            function metricValue(value) {
                if (value === null || value === undefined)
                    return Number.NaN

                if (typeof value === "number")
                    return value

                if (typeof value === "object") {
                    if (value.value !== undefined)
                        return metricValue(value.value)
                    if (value.distance !== undefined)
                        return metricValue(value.distance)
                    if (value.meters !== undefined)
                        return metricValue(value.meters)
                    if (value.text !== undefined)
                        return metricValue(value.text)
                }

                var text = String(value).replace(",", ".").trim()
                var match = text.match(/[-+]?[0-9]*\.?[0-9]+/)
                if (!match)
                    return Number.NaN

                var parsed = Number(match[0])
                if (!isFinite(parsed))
                    return Number.NaN

                if (/km/i.test(text))
                    return parsed * 1000
                return parsed
            }

            function stripInstructionMarkup(instruction) {
                var text = String(instruction || "")
                text = text.replace(/<[^>]+>/g, " ")
                text = text.replace(/&nbsp;/g, " ")
                text = text.replace(/&amp;/g, "&")
                text = text.replace(/&lt;/g, "<")
                text = text.replace(/&gt;/g, ">")
                text = text.replace(/\s+/g, " ")
                return text.trim()
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
                return text.trim()
            }

            function stepInstruction(step, index, count) {
                if (!step)
                    return index === count - 1 ? qsTr("Đến điểm đến") : qsTr("Tiếp tục")

                var maneuver = step.maneuver || {}
                var text = step.html_instructions || step.instruction || step.instructions
                        || maneuver.instructionText || maneuver.instruction || maneuver.type
                        || step.name || ""
                text = stripInstructionMarkup(text)
                if (text.length === 0) {
                    if (index === 0)
                        text = qsTr("Bắt đầu đi")
                    else if (index === count - 1)
                        text = qsTr("Đến điểm đến")
                    else
                        text = qsTr("Tiếp tục")
                }
                return localizeInstruction(text)
            }

            function distanceBetweenRoutePoints(a, b) {
                if (!coordValid(a) || !coordValid(b))
                    return 0

                var toRad = function(x) { return x * Math.PI / 180.0 }
                var earthRadius = 6371000.0
                var lat1 = toRad(a.latitude)
                var lat2 = toRad(b.latitude)
                var dLat = toRad(b.latitude - a.latitude)
                var dLon = toRad(b.longitude - a.longitude)
                var s = Math.sin(dLat / 2) * Math.sin(dLat / 2)
                        + Math.cos(lat1) * Math.cos(lat2)
                        * Math.sin(dLon / 2) * Math.sin(dLon / 2)
                return earthRadius * 2 * Math.atan2(Math.sqrt(s), Math.sqrt(1 - s))
            }

            function routePathDistance(points) {
                if (!points || points.length < 2)
                    return Number.NaN

                var distance = 0
                for (var i = 1; i < points.length; ++i)
                    distance += distanceBetweenRoutePoints(points[i - 1], points[i])
                return distance
            }

            function pointsFromGoongStep(step) {
                var pts = []
                if (!step)
                    return pts

                var encoded = stepPolyline(step)
                if (encoded.length > 0)
                    return decodeGoongPolyline(encoded)

                if (step.geometry && step.geometry.coordinates) {
                    var coords = step.geometry.coordinates
                    for (var i = 0; i < coords.length; ++i)
                        appendRoutePoint(pts, QtPositioning.coordinate(coords[i][1], coords[i][0]))
                }
                return pts
            }

            function pointsFromGoongRoute(route) {
                if (!route)
                    return []

                if (route.legs) {
                    var stepPts = []
                    for (var legIndex = 0; legIndex < route.legs.length; ++legIndex) {
                        var leg = route.legs[legIndex] || {}
                        var steps = leg.steps || []
                        for (var stepIndex = 0; stepIndex < steps.length; ++stepIndex) {
                            var step = steps[stepIndex] || {}
                            var stepEncoded = stepPolyline(step)
                            if (stepEncoded.length > 0) {
                                var decoded = decodeGoongPolyline(stepEncoded)
                                for (var j = 0; j < decoded.length; ++j)
                                    appendRoutePoint(stepPts, decoded[j])
                            } else if (step.geometry && step.geometry.coordinates) {
                                var stepCoords = step.geometry.coordinates
                                for (var k = 0; k < stepCoords.length; ++k)
                                    appendRoutePoint(stepPts, QtPositioning.coordinate(stepCoords[k][1], stepCoords[k][0]))
                            }
                        }
                    }
                    if (stepPts.length > 1)
                        return stepPts
                }

                var encoded = ""
                if (route.overview_polyline && route.overview_polyline.points)
                    encoded = route.overview_polyline.points
                else if (route.polyline)
                    encoded = route.polyline
                else if (route.geometry && typeof route.geometry === "string")
                    encoded = route.geometry

                if (encoded.length > 0)
                    return decodeGoongPolyline(encoded)

                if (route.geometry && route.geometry.coordinates) {
                    var coords = route.geometry.coordinates
                    var pts = []
                    for (var i = 0; i < coords.length; ++i)
                        appendRoutePoint(pts, QtPositioning.coordinate(coords[i][1], coords[i][0]))
                    return pts
                }

                return []
            }

            function guidanceStepsFromGoongRoute(route, routePoints) {
                var guidance = []
                var cumulativeMeters = 0

                if (route && route.legs) {
                    for (var legIndex = 0; legIndex < route.legs.length; ++legIndex) {
                        var leg = route.legs[legIndex] || {}
                        var steps = leg.steps || []
                        for (var stepIndex = 0; stepIndex < steps.length; ++stepIndex) {
                            var step = steps[stepIndex] || {}
                            var stepPoints = pointsFromGoongStep(step)
                            var distanceMeters = metricValue(step.distance)
                            var geometryDistance = routePathDistance(stepPoints)
                            if (!isFinite(distanceMeters) || distanceMeters <= 0)
                                distanceMeters = geometryDistance
                            if (!isFinite(distanceMeters) || distanceMeters <= 0)
                                distanceMeters = 1

                            var instruction = stepInstruction(step, guidance.length, steps.length)
                            guidance.push({
                                "startMeters": cumulativeMeters,
                                "endMeters": cumulativeMeters + distanceMeters,
                                "distanceMeters": distanceMeters,
                                "instruction": instruction
                            })
                            cumulativeMeters += distanceMeters
                        }
                    }
                }

                if (guidance.length === 0) {
                    var fallbackDistance = metricValue(route ? route.distance : Number.NaN)
                    if (!isFinite(fallbackDistance) || fallbackDistance <= 0)
                        fallbackDistance = routePathDistance(routePoints)
                    guidance.push({
                        "startMeters": 0,
                        "endMeters": isFinite(fallbackDistance) && fallbackDistance > 0 ? fallbackDistance : 1,
                        "distanceMeters": isFinite(fallbackDistance) && fallbackDistance > 0 ? fallbackDistance : 1,
                        "instruction": qsTr("Đi theo tuyến đường Goong")
                    })
                } else {
                    var last = guidance[guidance.length - 1]
                    if (!last.instruction || last.instruction === qsTr("Tiếp tục"))
                        last.instruction = qsTr("Đến điểm đến")
                }

                return guidance
            }

            function currentGuidanceStep(progressMeters) {
                if (!mapRouteGuidanceSteps || mapRouteGuidanceSteps.length === 0)
                    return null

                var progress = isFinite(progressMeters) ? Math.max(0, progressMeters) : 0
                for (var i = 0; i < mapRouteGuidanceSteps.length; ++i) {
                    var step = mapRouteGuidanceSteps[i]
                    if (progress < step.endMeters - 2)
                        return step
                }

                return mapRouteGuidanceSteps[mapRouteGuidanceSteps.length - 1]
            }

            function currentGuidanceInstruction(progressMeters) {
                var step = currentGuidanceStep(progressMeters)
                return step ? step.instruction : qsTr("Chưa có tuyến đường")
            }

            function currentGuidanceDistanceMeters(progressMeters) {
                var step = currentGuidanceStep(progressMeters)
                if (!step)
                    return Number.NaN

                var progress = isFinite(progressMeters) ? Math.max(0, progressMeters) : 0
                return Math.max(0, step.endMeters - progress)
            }

            function formatGuidanceDistance(meters) {
                if (!isFinite(meters))
                    return "-- m"

                var remaining = Math.max(0, meters)
                if (remaining >= 1000)
                    return (remaining / 1000).toFixed(remaining >= 10000 ? 0 : 1) + " km"
                return Math.round(remaining) + " m"
            }

            function resetClusterRouteState(reason, keepArrivalPanel) {
                mapRouteEpoch += 1
                mapNavigationSessionId += 1
                mapRouteRequestId = -1
                mapRouteRequestEpoch = -1

                mapRouteReady = false
                mapRouteProgressReady = false
                mapRoutePath = []
                mapRouteGuidanceSteps = []

                if (!keepArrivalPanel) {
                    mapArrivalHandled = false
                    mapArrivalPanelVisible = false
                    arrivalPanelTimer.stop()
                }

                if (clusterMapBackground)
                    clusterMapBackground.resetNavigationState()

                console.warn("CLUSTER_MAP_ROUTE_RESET", reason, "epoch", mapRouteEpoch)
            }

            function setClusterRoute(points, source, guidanceSteps) {
                if (mapArrivalHandled) {
                    console.warn("CLUSTER_MAP_ROUTE_SET_IGNORED_AFTER_ARRIVAL", source)
                    return
                }

                if (!points || points.length < 2) {
                    mapNavigationSessionId += 1
                    mapRouteRequestId = -1
                    mapRouteRequestEpoch = -1
                    mapRoutePath = []
                    mapRouteGuidanceSteps = []
                    mapRouteReady = false
                    mapRouteProgressReady = false
                    console.warn("CLUSTER_MAP_ROUTE_UNAVAILABLE", source)

                    if (clusterMapBackground)
                        clusterMapBackground.resetNavigationState()

                    return
                }

                mapRouteProgressReady = false
                mapRoutePath = points
                mapRouteGuidanceSteps = guidanceSteps || []
                mapRouteReady = true
                mapArrivalHandled = false
                mapArrivalPanelVisible = false
                console.warn("CLUSTER_MAP_ROUTE_READY", source, "points", points.length, "epoch", mapRouteEpoch)

                if (gear_mode === "D")
                    clusterMapBackground.activateFirstPersonDrive()
            }

            function startDriveMapWhenRouteReady() {
                if (mapArrivalHandled) {
                    console.warn("CLUSTER_MAP_START_IGNORED_AFTER_ARRIVAL")
                    if (clusterMapBackground)
                        clusterMapBackground.resetNavigationState()
                    return
                }

                if (mapRoutePath && mapRoutePath.length > 1) {
                    clusterMapBackground.activateFirstPersonDrive()
                    return
                }

                requestClusterMapRoute("gear-D")
            }

            function requestClusterMapRoute(reason) {
                if (!coordValid(mapDriveStartCoordinate) || !coordValid(mapDriveEndCoordinate)) {
                    console.warn("CLUSTER_MAP_ROUTE_INVALID_COORDS",
                                 "start", mapDriveStartCoordinate,
                                 "end", mapDriveEndCoordinate)
                    resetClusterRouteState("invalid-coords", false)
                    return
                }

                mapRouteEpoch += 1
                mapNavigationSessionId += 1
                mapRouteRequestId = -1
                mapRouteRequestEpoch = mapRouteEpoch

                mapRouteReady = false
                mapRouteProgressReady = false
                mapRoutePath = []
                mapRouteGuidanceSteps = []
                mapArrivalHandled = false
                mapArrivalPanelVisible = false
                arrivalPanelTimer.stop()

                if (clusterMapBackground)
                    clusterMapBackground.resetNavigationState()

                if (typeof GoongRestClient === "undefined" || GoongRestClient === null || !GoongRestClient.available) {
                    console.warn("CLUSTER_MAP_ROUTE_UNAVAILABLE: missing GOONG_REST_API_KEY")
                    setClusterRoute([], "missing-key")
                    return
                }

                mapRouteRequestId = GoongRestClient.direction(mapDriveStartCoordinate.latitude,
                                                            mapDriveStartCoordinate.longitude,
                                                            mapDriveEndCoordinate.latitude,
                                                            mapDriveEndCoordinate.longitude,
                                                            "car")
                mapRouteRequestEpoch = mapRouteEpoch

                console.warn("CLUSTER_MAP_ROUTE_REQUEST",
                             "id", mapRouteRequestId,
                             "epoch", mapRouteRequestEpoch,
                             "reason", reason || "manual")
            }

            function setPhoneNavigationDestination(lat, lon, name, startLat, startLon, hasStart) {
                var dest = QtPositioning.coordinate(lat, lon)

                if (!coordValid(dest)) {
                    console.warn("PHONE_NAV_DEST_INVALID", lat, lon, name)
                    return
                }

                if (hasStart) {
                    var start = QtPositioning.coordinate(startLat, startLon)
                    if (coordValid(start))
                        mapDriveStartCoordinate = start
                }

                console.warn("PHONE_NAV_DEST_RECEIVED",
                             dest.latitude, dest.longitude,
                             name || "",
                             "start", mapDriveStartCoordinate.latitude, mapDriveStartCoordinate.longitude)

                mapDriveEndCoordinate = dest
                mapPhoneDestinationName = name || ""
                mapArrivalHandled = false
                mapArrivalPanelVisible = false
                arrivalPanelTimer.stop()

                if (clusterMapBackground)
                    clusterMapBackground.resetNavigationState()

                requestClusterMapRoute("phone-destination")
            }

            function completeClusterRoute() {
                if (mapArrivalHandled)
                    return

                console.warn("CLUSTER_MAP_ROUTE_COMPLETE",
                             "requestId", mapRouteRequestId,
                             "epoch", mapRouteEpoch)

                mapArrivalHandled = true
                mapArrivalPanelVisible = true
                arrivalPanelTimer.restart()

                mapRouteEpoch += 1
                mapNavigationSessionId += 1
                mapRouteRequestId = -1
                mapRouteRequestEpoch = -1
                mapRouteReady = false
                mapRouteProgressReady = false
                mapRoutePath = []
                mapRouteGuidanceSteps = []

                if (clusterMapBackground)
                    clusterMapBackground.resetNavigationState()
            }

            function markRouteProgressReady() {
                if (!mapRouteReady || !mapRoutePath || mapRoutePath.length < 2)
                    return

                if (!clusterMapBackground || clusterMapBackground.routeTotalMeters <= 0)
                    return

                mapRouteProgressReady = true
                handleRouteProgressChanged()
            }

            function handleRouteProgressChanged() {
                if (mapArrivalHandled || !mapRouteReady || !mapRouteProgressReady
                        || !mapRoutePath || mapRoutePath.length < 2)
                    return

                if (!clusterMapBackground || clusterMapBackground.routeTotalMeters <= 0)
                    return

                if (clusterMapBackground.routeRemainingMeters <= 3
                        || clusterMapBackground.routeProgressMeters >= clusterMapBackground.routeTotalMeters - 1)
                    completeClusterRoute()
            }

            function s(value) {
                return Math.round(value * uiScale)
            }

            function formatRouteDistance(meters, totalMeters) {
                if (!isFinite(totalMeters) || totalMeters <= 0 || !isFinite(meters))
                    return "-- KM"

                var remaining = Math.max(0, meters)
                if (remaining >= 10000)
                    return Math.round(remaining / 1000) + " KM"
                if (remaining >= 1000)
                    return (remaining / 1000).toFixed(1) + " KM"
                return Math.round(remaining) + " M"
            }

            function formatRouteTime(seconds, totalMeters) {
                if (!isFinite(totalMeters) || totalMeters <= 0 || !isFinite(seconds) || seconds < 0)
                    return "-- MIN"

                if (seconds <= 5)
                    return "0 MIN"

                var minutes = Math.max(1, Math.ceil(seconds / 60))
                if (minutes < 60)
                    return minutes + " MIN"

                var hours = Math.floor(minutes / 60)
                var rest = minutes % 60
                return rest > 0 ? (hours + "H " + rest + "M") : (hours + "H")
            }

            function startupClamp01(value) {
                if (value <= 0)
                    return 0
                if (value >= 1)
                    return 1
                return value
            }

            function startupPulse(index) {
                var local = (startupBlink + index * 0.073) % 1
                return local < 0.5 ? local * 2 : (1 - local) * 2
            }

            function startupReveal(index) {
                if (startupPhase === 0)
                    return 0
                if (startupPhase === 2)
                    return 1

                return startupClamp01((startupSweep - index * 0.085) / 0.18)
            }

            function startupPanelOpacity(index) {
                if (startupPhase === 0)
                    return 0
                if (startupPhase === 2)
                    return 1

                return 0.82 + startupReveal(index) * 0.18
            }

            function startupOpacity(index) {
                if (startupPhase === 0)
                    return 0
                if (startupPhase === 2)
                    return 1

                var reveal = startupReveal(index)
                if (reveal <= 0)
                    return 0

                return reveal * (0.46 + startupPulse(index) * 0.54)
            }

            function startupScale(index) {
                if (startupPhase !== 1)
                    return 1

                var reveal = startupReveal(index)
                if (reveal <= 0)
                    return 0.96

                return 0.96 + reveal * 0.025 + startupPulse(index) * 0.015
            }

            function finishStartupIfReady() {
                if (startupPhase === 1 && startupComponentsReady && startupHoldTimer.readyToFinish) {
                    startupPhase = 2
                    startupSelfCheckDone = true
                    window.clusterReadyToShow = true
                    startupReadySignalTimer.start()
                }
            }

            function handleStartupReadinessChanged() {
                finishStartupIfReady()
            }

            function beginStartupSelfCheck() {
                if (startupSelfCheckStarted || startupPhase !== 0)
                    return

                startupSelfCheckStarted = true
                console.log("myCluster startup: start cluster self-check")
                startupBlink = 0
                startupSweep = 0
                startupHoldTimer.readyToFinish = false
                startupPhase = 1
            }

            function markStaticUiReadyIfReady() {
                if (startupStaticReadySignalSent || !startupStaticComponentsReady)
                    return

                if (startupPhase < 1)
                    return

                startupStaticReadySignalSent = true
                if (typeof startupNotifier !== "undefined" && startupNotifier !== null)
                    startupNotifier.markUiReady()

                start3DLoadIfReady()
            }

            function start3DLoadIfReady() {
                if (!root.allow3DModel) {
                    handleStartupReadinessChanged()
                    return
                }

                if (root.load3DModel || load3DTimer.running)
                    return

                load3DTimer.start()
            }

            function moveGearIndicator(button) {
                var point = line_devide.parent.mapFromItem(button, button.width / 2, 0)
                line_devide.visible = true
                line_devide.x = point.x - line_devide.width / 2
            }

            function activeModelItem() {
                return carViewLoader.item
            }

            function applyCarViewState() {
                var item = activeModelItem()
                if (!item)
                    return

                item.setKeyFrame(pendingCarKeyFrame)
                if (pendingRearCamera)
                    item.showRearView()
                else
                    item.showFrontView()
            }

            function showCarFrontView() {
                pendingCarKeyFrame = 360
                pendingRearCamera = false
                applyCarViewState()
            }

            function showCarRearView() {
                pendingCarKeyFrame = 0
                pendingRearCamera = true
                applyCarViewState()
            }

            function traceInput(source, x, y) {
                console.log("myCluster input", source, Math.round(x), Math.round(y), "active", root.active, "visibility", root.visibility)
            }

            function switchToSerialMode() {
                if (demoMode)
                    demoMode = false
            }

            function traceSerialUi(name, value) {
                if (serialTraceValues[name] === value)
                    return

                serialTraceValues[name] = value
                console.log("myCluster QML rx", name + "=" + value, "demoMode=" + demoMode)
            }

            property double startTime: Date.now()
            property int elapsedMinutes: 0
            property string gear_mode: "P"
            onGear_modeChanged: {
                if (gear_mode === "D")
                    startDriveMapWhenRouteReady()
                else if (clusterMapBackground)
                    clusterMapBackground.stopDriveAnimation()
            }

            Timer {
                id: timer
                interval: 1000
                running: true
                repeat: true
                onTriggered: {
                    var now = Date.now();
                    root.elapsedMinutes = Math.floor((now - startTime) / 60000);
                }
            }
            Timer {
                id: arrivalPanelTimer
                interval: 5000
                repeat: false
                onTriggered: root.mapArrivalPanelVisible = false
            }
            Timer {
                id: load3DTimer
                interval: root.load3DDelayMs
                running: false
                repeat: false
                onTriggered: root.load3DModel = root.allow3DModel
            }
            Timer {
                id: startupLogoTimer
                interval: 1050
                running: true
                repeat: false
                onTriggered: root.beginStartupSelfCheck()
            }
            Timer {
                id: startupHoldTimer
                property bool readyToFinish: false
                interval: 2400
                repeat: false
                onTriggered: {
                    readyToFinish = true
                    root.handleStartupReadinessChanged()
                }
            }
            Timer {
                id: startupMaxTimer
                interval: 30000
                repeat: false
                onTriggered: {
                    if (root.startupComponentsReady) {
                        root.startupPhase = 2
                        root.startupSelfCheckDone = true
                        startupReadySignalTimer.start()
                    } else {
                        console.warn("myCluster startup still waiting",
                                    "staticReady=" + root.startupStaticComponentsReady,
                                    "modelReady=" + root.startupModelReady,
                                    "loaderStatus=" + carViewLoader.status)
                        restart()
                    }
                }
            }
            Timer {
                id: startupReadySignalTimer
                interval: 40
                repeat: false
                onTriggered: {
                    if (root.startupReadySignalSent)
                        return

                    root.startupReadySignalSent = true
                    if (typeof startupNotifier !== "undefined" && startupNotifier !== null)
                        startupNotifier.markReady()
                }
            }
            NumberAnimation on startupBlink {
                running: root.startupSelfCheckActive
                loops: Animation.Infinite
                from: 0
                to: 1
                duration: 920
            }
            NumberAnimation on startupSweep {
                running: root.startupSelfCheckActive && root.startupSweep < 1
                loops: 1
                from: 0
                to: 1
                duration: 1750
                easing.type: Easing.OutCubic
            }
            onStartupPhaseChanged: {
                if (startupPhase === 1) {
                    startupHoldTimer.readyToFinish = false
                    startupHoldTimer.start()
                    startupMaxTimer.start()
                    markStaticUiReadyIfReady()
                }
            }
            onStartupStaticComponentsReadyChanged: markStaticUiReadyIfReady()
            onStartupComponentsReadyChanged: handleStartupReadinessChanged()
            ButtonGroup {
                id: gearGroup
                exclusive: true
            }

            Connections {
                target: GoongRestClient

                function onReplyReady(requestId, api, payload) {
                    if (api !== "direction"
                            || requestId !== root.mapRouteRequestId
                            || root.mapRouteRequestEpoch !== root.mapRouteEpoch
                            || root.mapArrivalHandled) {
                        console.warn("CLUSTER_MAP_ROUTE_REPLY_IGNORED",
                                     "requestId", requestId,
                                     "currentId", root.mapRouteRequestId,
                                     "replyEpoch", root.mapRouteRequestEpoch,
                                     "currentEpoch", root.mapRouteEpoch,
                                     "arrival", root.mapArrivalHandled)
                        return
                    }

                    var routes = payload && payload.routes ? payload.routes : []
                    if (!routes || routes.length === 0) {
                        console.warn("CLUSTER_MAP_ROUTE_EMPTY")
                        root.setClusterRoute([], "empty-route")
                        return
                    }

                    var points = root.pointsFromGoongRoute(routes[0])
                    var guidanceSteps = root.guidanceStepsFromGoongRoute(routes[0], points)
                    root.setClusterRoute(points, "goong", guidanceSteps)
                }

                function onRequestFailed(requestId, api, message) {
                    if (api !== "direction"
                            || requestId !== root.mapRouteRequestId
                            || root.mapRouteRequestEpoch !== root.mapRouteEpoch
                            || root.mapArrivalHandled) {
                        console.warn("CLUSTER_MAP_ROUTE_FAIL_IGNORED",
                                     "requestId", requestId,
                                     "currentId", root.mapRouteRequestId,
                                     "replyEpoch", root.mapRouteRequestEpoch,
                                     "currentEpoch", root.mapRouteEpoch,
                                     "arrival", root.mapArrivalHandled)
                        return
                    }

                    console.warn("CLUSTER_MAP_ROUTE_FAILED", message)
                    root.setClusterRoute([], "request-failed")
                }
            }

            Connections {
                target: typeof phoneNavigationBridge === "undefined" ? null : phoneNavigationBridge

                function onDestinationReceived(lat, lon, name, startLat, startLon, hasStart) {
                    root.setPhoneNavigationDestination(lat, lon, name, startLat, startLon, hasStart)
                }

                function onParseFailed(message, rawBody) {
                    console.warn("PHONE_NAV_PARSE_FAILED_QML", message, rawBody)
                }

                function onRequestReceived(path, body) {
                    console.warn("PHONE_NAV_REQUEST_QML", path, body)
                }
            }

            Component.onCompleted: {
                window.raise()
                window.requestActivate()
                root.requestClusterMapRoute("startup")
            }

        
            Rectangle{
                width: parent.width
                height: parent.height
                z: -2
                color: "black"
            }

            Image {
                id: backgroundPanel
                width: 1640; height: 1060
                //anchors.fill:
                //anchors.centerIn: 
                enabled: false
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenterOffset: 0
                source: "qrc:/asset/Panel_2.png"
                //fillMode: Image.PreserveAspectCrop
                smooth: true
                z: 1
                opacity: root.startupPanelOpacity(0)
                Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
            }

            Image{
                id: panel
                z: 0
                opacity: root.startupPanelOpacity(0.25)
                Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                sourceSize: Qt.size(1260, 800)//* 0.8)
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenterOffset: -25
                source: "qrc:/asset/Panel.png"
                clip: true

                ClusterMapLibre {
                    id: clusterMapBackground
                    anchors.fill: parent
                    z: 0
                    opacity: root.gear_mode === "D" ? root.startupPanelOpacity(0.4) : 0
                    enabled: root.gear_mode === "D"
                    Behavior on opacity { NumberAnimation { duration: 110; easing.type: Easing.OutQuad } }
                    centerCoordinate: root.mapDriveStartCoordinate
                    zoomLevel: 18.25
                    pitch: 74
                    bearing: -18
                    map3dEnabled: true
                    driveStartCoordinate: root.mapDriveStartCoordinate
                    driveEndCoordinate: root.mapDriveEndCoordinate
                    routePath: root.mapRoutePath
                    driveSpeedKph: root.gear_mode === "D" ? root.mapDriveSpeedKph : 0
                    stoppedSpeedThresholdKph: 0.5
                    driveAnimationEnabled: root.mapRoutePath && root.mapRoutePath.length > 1
                    navigationSessionId: root.mapNavigationSessionId
                    firstPersonZoom: 18.25
                    firstPersonPitch: 74
                    buildingSourceLayer: "VN_Building"
                    goongMapTilesKey: typeof GoongMapTilesKey !== "undefined" && String(GoongMapTilesKey).length > 0
                                    ? GoongMapTilesKey
                                    : "Xgw2Eiqb38KmKuSsxQFH5c4NtEuORfbAdWfbxXIB"
                    goongStyleUrl: typeof GoongStyleUrl !== "undefined" && String(GoongStyleUrl).length > 0
                                ? GoongStyleUrl
                                : "https://tiles.goong.io/assets/goong_map_web.json"
                    onRouteRemainingMetersChanged: root.markRouteProgressReady()
                }

                IconButton{
                    id:leftIndicator
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.0)
                    scale: root.startupScale(1.0)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-left-checked/icon-park-solid_right-two.svg" : ""//qrc:/asset/icons/icons-left/icon-park-solid_right-two.svg
                    anchors.right: topBar.left
                    anchors.rightMargin: root.s(24)
                    anchors.verticalCenter: topBar.verticalCenter
                    SequentialAnimation {
                        running: leftIndicator.checked || root.startupSelfCheckActive
                        loops: Animation.Infinite
                        OpacityAnimator {
                            target: leftIndicator.roundIcon ? leftIndicator.roundIconSource : leftIndicator.iconSource
                            from: 0;
                            to: 1;
                            duration: 500
                        }
                        OpacityAnimator {
                            target: leftIndicator.roundIcon ? leftIndicator.roundIconSource : leftIndicator.iconSource
                            from: 1;
                            to: 0;
                            duration: 500
                        }
                    }
                    onCheckedChanged:{
                        Cluster.light_hazard = checked
                    }
                }
                IconButton{
                    id:leftIndicator1
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.0)
                    scale: root.startupScale(1.0)
                    checkable: true
                    visible: false
                    setIcon:checked ? "qrc:/asset/icons/icons-left-checked/icon-park-solid_right-two.svg" : ""//qrc:/asset/icons/icons-left/icon-park-solid_right-two.svg
                    anchors.right: topBar.left
                    anchors.rightMargin: root.s(24)
                    anchors.verticalCenter: topBar.verticalCenter
                    SequentialAnimation {
                        running: leftIndicator1.checked
                        loops: Animation.Infinite
                        OpacityAnimator {
                            target: leftIndicator1.roundIcon ? leftIndicator1.roundIconSource : leftIndicator1.iconSource
                            from: 0;
                            to: 1;
                            duration: 500
                        }
                        OpacityAnimator {
                            target: leftIndicator1.roundIcon ? leftIndicator1.roundIconSource : leftIndicator1.iconSource
                            from: 1;
                            to: 0;
                            duration: 500
                        }
                    }
                }
                IconButton{
                    id:handbreak
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.12)
                    scale: root.startupScale(1.12)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-left/mdi_car-handbrake.svg" : ""//qrc:/asset/icons/icons-left/icons8-brake-warning-32.png
                    anchors{
                        right: leftIndicator.left
                        rightMargin: root.s(8)
                        verticalCenter: leftIndicator.verticalCenter
                        verticalCenterOffset: root.s(22)
                    }
                    SequentialAnimation {
                        running: handbreak.checked || root.startupSelfCheckActive
                        loops: Animation.Infinite
                        OpacityAnimator {
                            target: handbreak.roundIcon ? handbreak.roundIconSource : handbreak.iconSource
                            from: 0;
                            to: 1;
                            duration: 500
                        }
                        OpacityAnimator {
                            target: handbreak.roundIcon ? handbreak.roundIconSource : handbreak.iconSource
                            from: 1;
                            to: 0;
                            duration: 500
                        }
                    }
                }
                IconButton{
                    id:battery
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.24)
                    scale: root.startupScale(1.24)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-left-checked/mdi_car-battery.svg" : ""//qrc:/asset/icons/icons-left/mdi_car-battery.svg
                    anchors{
                        right: handbreak.left
                        rightMargin: root.s(8)
                        verticalCenter: handbreak.verticalCenter
                        verticalCenterOffset: root.s(22)
                    }
                    SequentialAnimation {
                        running: battery.checked || root.startupSelfCheckActive
                        loops: Animation.Infinite
                        OpacityAnimator {
                            target: battery.roundIcon ? battery.roundIconSource : battery.iconSource
                            from: 0;
                            to: 1;
                            duration: 500
                        }
                        OpacityAnimator {
                            target: battery.roundIcon ? battery.roundIconSource : battery.iconSource
                            from: 1;
                            to: 0;
                            duration: 500
                        }
                    }
                }
                IconButton{
                    id:engineBold
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.36)
                    scale: root.startupScale(1.36)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-left-checked/ph_engine-bold.svg" : ""//qrc:/asset/icons/icons-left/ph_engine-bold.svg
                    anchors{
                        right: battery.left
                        rightMargin: root.s(6)
                        verticalCenter: battery.verticalCenter
                        verticalCenterOffset: root.s(24)
                    }
                    SequentialAnimation {
                        running: engineBold.checked || root.startupSelfCheckActive
                        loops: Animation.Infinite
                        OpacityAnimator {
                            target: engineBold.roundIcon ? engineBold.roundIconSource : engineBold.iconSource
                            from: 0;
                            to: 1;
                            duration: 500
                        }
                        OpacityAnimator {
                            target: engineBold.roundIcon ? engineBold.roundIconSource : engineBold.iconSource
                            from: 1;
                            to: 0;
                            duration: 500
                        }
                    }
                }
                IconButton{
                    id:oil
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.48)
                    scale: root.startupScale(1.48)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-left-checked/mdi_oil.svg" : ""//qrc:/asset/icons/icons-left/mdi_oil.svg
                    anchors{
                        right: engineBold.left
                        rightMargin: root.s(6)
                        verticalCenter: engineBold.verticalCenter
                        verticalCenterOffset: root.s(26)
                    }
                    SequentialAnimation {
                        running: oil.checked || root.startupSelfCheckActive
                        loops: Animation.Infinite
                        OpacityAnimator {
                            target: oil.roundIcon ? oil.roundIconSource : oil.iconSource
                            from: 0;
                            to: 1;
                            duration: 500
                        }
                        OpacityAnimator {
                            target: oil.roundIcon ? oil.roundIconSource : oil.iconSource
                            from: 1;
                            to: 0;
                            duration: 500
                        }
                    }
                }
                IconButton{
                    id:tireAlert
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.6)
                    scale: root.startupScale(1.6)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-left/mdi_car-tire-alert.svg" : ""//qrc:/asset/icons/icons-left/icons8-tire-pressure-24.png
                    anchors{
                        right: oil.left
                        verticalCenter: oil.verticalCenter
                        verticalCenterOffset: root.s(30)
                    }
                    SequentialAnimation {
                        running: tireAlert.checked || root.startupSelfCheckActive
                        loops: Animation.Infinite
                        OpacityAnimator {
                            target: tireAlert.roundIcon ? tireAlert.roundIconSource : tireAlert.iconSource
                            from: 0;
                            to: 1;
                            duration: 500
                        }
                        OpacityAnimator {
                            target: tireAlert.roundIcon ? tireAlert.roundIconSource : tireAlert.iconSource
                            from: 1;
                            to: 0;
                            duration: 500
                        }
                    }
                }

                Image{
                    id:topBar
                    source: "qrc:/asset/Top Bar.png"
                    sourceSize: Qt.size(root.topBarWidth, root.topBarHeight)
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    opacity: root.startupOpacity(0.45)
                    scale: root.startupScale(0.45)
                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }


                    RowLayout{
                        anchors.left: parent.left
                        anchors.leftMargin: root.s(100)
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: root.s(7)
                        Image{
                            source: "qrc:/asset/icons/cloud.svg"
                            sourceSize: Qt.size(root.s(20), root.s(20))
                        }
                        Label{
                            text: qsTr("25 °C")
                            font.pixelSize: root.s(20)
                            font.bold: true
                            font.weight: Font.Normal
                            color: "#FFFFFF"
                            font.family: root.textFontFamily
                        }
                    }

                    Label{
                        id:timeLabel
                        text: new Date().toLocaleTimeString(Qt.locale(), "hh:mm AP")
                        anchors.right: parent.right
                        anchors.rightMargin: root.s(100)
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: root.s(20)
                        font.bold: true
                        font.weight: Font.Normal
                        font.family: root.textFontFamily
                        color: "#FFFFFF"
                    }
                }

                IconButton{
                    id:rightIndicator
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.0)
                    scale: root.startupScale(1.0)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-right-checked/icon-park-solid_right-two.svg" : ""//qrc:/asset/icons/icons-right/icon-park-solid_right-two.svg
                    anchors.left: topBar.right
                    anchors.leftMargin: root.s(24)
                    anchors.verticalCenter: topBar.verticalCenter
                    SequentialAnimation {
                        running: rightIndicator.checked || root.startupSelfCheckActive
                        loops: Animation.Infinite
                        OpacityAnimator {
                            target: rightIndicator.roundIcon ? rightIndicator.roundIconSource : rightIndicator.iconSource
                            from: 0;
                            to: 1;
                            duration: 500
                        }
                        OpacityAnimator {
                            target: rightIndicator.roundIcon ? rightIndicator.roundIconSource : rightIndicator.iconSource
                            from: 1;
                            to: 0;
                            duration: 500
                        }
                    }
                }
                IconButton{
                    id:rightIndicator1
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.0)
                    scale: root.startupScale(1.0)
                    checkable: true
                    setIcon:checked ? "qrc:/asset/icons/icons-right-checked/icon-park-solid_right-two.svg" : ""//qrc:/asset/icons/icons-right/icon-park-solid_right-two.svg
                    anchors.left: topBar.right
                    anchors.leftMargin: root.s(24)
                    anchors.verticalCenter: topBar.verticalCenter
                    visible: false
                    SequentialAnimation {
                        running: rightIndicator1.checked
                        loops: Animation.Infinite
                        OpacityAnimator {
                            target: rightIndicator1.roundIcon ? rightIndicator1.roundIconSource : rightIndicator1.iconSource
                            from: 0;
                            to: 1;
                            duration: 500
                        }
                        OpacityAnimator {
                            target: rightIndicator1.roundIcon ? rightIndicator1.roundIconSource : rightIndicator1.iconSource
                            from: 1;
                            to: 0;
                            duration: 500
                        }
                    }
                }
                IconButton{
                    id:seatBreak
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.12)
                    scale: root.startupScale(1.12)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-right/mdi_seatbelt.svg" : ""//qrc:/asset/icons/icons-right/mdi_seatbelt.svg
                    anchors{
                        left: rightIndicator.right
                        leftMargin: root.s(8)
                        verticalCenter: rightIndicator.verticalCenter
                        verticalCenterOffset: root.s(22)
                    }
                    SequentialAnimation {
                        running: seatBreak.checked || root.startupSelfCheckActive
                        loops: Animation.Infinite
                        OpacityAnimator {
                            target: seatBreak.roundIcon ? seatBreak.roundIconSource : seatBreak.iconSource
                            from: 0;
                            to: 1;
                            duration: 500
                        }
                        OpacityAnimator {
                            target: seatBreak.roundIcon ? seatBreak.roundIconSource : seatBreak.iconSource
                            from: 1;
                            to: 0;
                            duration: 500
                        }
                    }
                }
                IconButton{
                    id:breakParking
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.24)
                    scale: root.startupScale(1.24)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-right/mdi_car-brake-parking.svg" : ""//qrc:/asset/icons/icons-right/car-brake-parking.svg
                    anchors{
                        left: seatBreak.right
                        leftMargin: root.s(8)
                        verticalCenter: seatBreak.verticalCenter
                        verticalCenterOffset: root.s(22)
                    }
                    SequentialAnimation {
                        running: breakParking.checked || root.startupSelfCheckActive
                        loops: Animation.Infinite
                        OpacityAnimator {
                            target: breakParking.roundIcon ? breakParking.roundIconSource : breakParking.iconSource
                            from: 0;
                            to: 1;
                            duration: 500
                        }
                        OpacityAnimator {
                            target: breakParking.roundIcon ? breakParking.roundIconSource : breakParking.iconSource
                            from: 1;
                            to: 0;
                            duration: 500
                        }
                    }
                }
                IconButton{
                    id:lightDimmed
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.36)
                    scale: root.startupScale(1.36)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-right/mdi_car-light-dimmed.svg" : ""//qrc:/asset/icons/icons-right/car-light-dimmed.svg
                    anchors{
                        left: breakParking.right
                        leftMargin: root.s(6)
                        verticalCenter: breakParking.verticalCenter
                        verticalCenterOffset: root.s(24)
                    }
                    onCheckedChanged: {
                        Cluster.light_dim = checked
                    }
                }
                IconButton{
                    id:lightHigh
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.48)
                    scale: root.startupScale(1.48)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-right-checked/mdi_car-light-high.svg" : ""//qrc:/asset/icons/icons-right/mdi_car-light-high.svg
                    anchors{
                        left: lightDimmed.right
                        leftMargin: root.s(6)
                        verticalCenter: lightDimmed.verticalCenter
                        verticalCenterOffset: root.s(26)
                    }
                    onCheckedChanged: {
                        Cluster.light_on = checked
                    }
                }
                IconButton{
                    id:lightFog
                    roundIcon: true
                    width: root.warningButtonSize
                    height: root.warningButtonSize
                    iconWidth: root.warningIconSize
                    iconHeight: root.warningIconSize
                    opacity: root.startupOpacity(1.6)
                    scale: root.startupScale(1.6)
                    checkable: true
                    setIcon:(checked || root.startupSelfCheckActive) ? "qrc:/asset/icons/icons-right/mdi_car-light-fog.svg" : ""//qrc:/asset/icons/icons-right/car-light-fog.svg
                    anchors{
                        left: lightHigh.right
                        verticalCenter: lightHigh.verticalCenter
                        verticalCenterOffset: root.s(30)
                    }
                    onCheckedChanged: {
                        Cluster.light_fog = checked
                    }
                }

                Image{
                    id:leftgauge
                    sourceSize: Qt.size(root.gaugeSize, root.gaugeSize)
                    anchors.left: parent.left
                    anchors.leftMargin: root.gaugeSideMargin
                    anchors.verticalCenterOffset: root.gaugeYOffset
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/asset/Tacometer.png"
                    opacity: root.startupOpacity(2.15)
                    scale: root.startupScale(2.15)
                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    CircularGaugeMeter {
                        id:leftGauge
                        z: -1
                        property int speed: 0
                        property bool accelerating
                        anchors.centerIn: parent
                        width: parent.width * 0.65
                        height: parent.height * 0.65
                        value: speed / 4//accelerating ? maximumValue : 0
                        shadowVisible: true
                        maximumValue: 80
                        Component.onCompleted: forceActiveFocus()
                        Behavior on value { NumberAnimation { duration: 300 }}
                        //NumberAnimation on speed { duration: 5000; easing.type: Easing.InOutQuad; from: 0; to: 240; loops: Animation.Infinite; running: root.demoMode }
                        Keys.onSpacePressed:{
                            accelerating = true
                            rightGauge.accelerating = true
                        }
                        Keys.onReleased: {
                            if (event.key === Qt.Key_Space) {
                                accelerating = false;
                                event.accepted = true;
                                rightGauge.accelerating = false
                                event.accepted = true;
                            }
                        }
                        Text{
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: -root.s(24)
                            font.bold: Font.DemiBold
                            font.weight: Font.Normal
                            font.pixelSize: root.s(38)
                            color: "#FFFFFF"
                            font.family: root.textFontFamily
                            text: Math.floor(leftGauge.speed/4) //Math.floor((leftIndi.value.toFixed(0)/30) *10)
                        }

                        Text{
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: root.s(22)
                            font.pixelSize: root.s(13)
                            font.bold: Font.DemiBold
                            font.weight: Font.Normal
                            color: "#FFFFFF"
                            font.family: root.textFontFamily
                            text: "RPM/100"
                        }
                    }
                }

                RowLayout{
                    id: navigationPrompt
                    z: 2
                    visible: gear_mode === "D"
                    width: topBar.width * 0.54
                    anchors.top: topBar.bottom
                    anchors.horizontalCenter: topBar.horizontalCenter
                    anchors.horizontalCenterOffset: root.s(32)
                    opacity: root.startupOpacity(3.1)
                    scale: root.startupScale(3.1)
                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    Item{
                        Layout.fillWidth: true
                    }

                    Image{
                        id: navigationTurnIcon
                        Layout.alignment: Qt.AlignHCenter
                        source: "qrc:/asset/icons/Road/mdi_turn-right-bold.svg"
                        sourceSize: Qt.size(root.s(62), root.s(62))
                    }

                    ColumnLayout{
                        Layout.alignment: Qt.AlignHCenter
                        Text{
                            id: navigationDistanceText
                            font.pixelSize: root.s(24)
                            font.bold: true
                            font.weight: Font.Normal
                            font.family: root.textFontFamily
                            color: "#FFFFFF"
                            text: root.formatGuidanceDistance(
                                    root.currentGuidanceDistanceMeters(clusterMapBackground.routeProgressMeters))
                        }
                        Text{
                            id: navigationInstructionText
                            Layout.preferredWidth: root.s(190)
                            font.pixelSize: root.s(15)
                            font.bold: true
                            font.weight: Font.Normal
                            font.family: root.textFontFamily
                            color: "#00D1FF"
                            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                            maximumLineCount: 2
                            elide: Text.ElideRight
                            text: root.currentGuidanceInstruction(clusterMapBackground.routeProgressMeters)
                        }
                        Item{
                            Layout.preferredHeight: root.s(18)
                        }
                    }
                    Item{
                        Layout.fillWidth: true
                    }
                }

                // Image{
                //     id: steeringAssistIcon
                //     z: 2
                //     source: "qrc:/asset/icons/Road/mingcute_steering-wheel-fill.svg"
                //     sourceSize: Qt.size(root.s(66), root.s(66))
                //     anchors.top: topBar.bottom
                //     anchors.horizontalCenter: topBar.horizontalCenter
                //     anchors.horizontalCenterOffset: root.s(180)
                //     anchors.topMargin: root.s(8)
                //     opacity: root.startupOpacity(3.3)
                //     scale: root.startupScale(3.3)
                //     Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                //     Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                // }

                // Image {
                //     id: roadImage
                //     visible: root.gear_mode !== "P" && root.gear_mode !== "N"
                //     anchors.centerIn: parent
                //     source: "qrc:/asset/icons/Road/road2.png"
                //     anchors.verticalCenterOffset: root.s(38)
                //     sourceSize.height: root.s(500)
                //     opacity: root.startupOpacity(4.1)
                //     scale: root.startupScale(4.1)
                //     Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                //     Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                // }

                ////////////////////////////////////////////////////////////////////////////////////////////////////////

                Rectangle{
                    id: modelCar
                    height: root.gear_mode === "P" || root.gear_mode === "N" ? root.modelParkHeight : root.modelDriveHeight
                    width: root.gear_mode === "P" || root.gear_mode === "N" ? root.modelParkWidth : root.modelDriveWidth
                    color:"transparent"
                    visible: true
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenterOffset: root.gear_mode === "P" || root.gear_mode === "N" ? -root.s(12) : root.s(132)
                    anchors.horizontalCenterOffset: root.gear_mode === "P" || root.gear_mode === "N" ? -root.s(24) : root.s(18)
                    opacity: root.startupOpacity(4.35)
                    scale: root.startupScale(4.35)
                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    Image {
                        anchors.centerIn: parent
                        width: parent.width * (root.gear_mode === "P" || root.gear_mode === "N" ? 0.58 : 0.5)
                        height: parent.height * (root.gear_mode === "P" || root.gear_mode === "N" ? 0.58 : 0.5)
                        source: "qrc:/asset/icons/Road/Model-3-2.png"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        visible: carViewLoader.status !== Loader.Ready
                    }
                    Loader {
                        id: carViewLoader
                        anchors.fill: parent
                        active: root.load3DModel
                        asynchronous: true
                        enabled: false
                        source: "QML/CarView3D.qml"
                        visible: status === Loader.Ready && root.gear_mode === "P"
                        onLoaded: {
                            root.applyCarViewState()
                            root.startupModelSettled = item.modelReady
                            if (root.startupModelSettled)
                                root.handleStartupReadinessChanged()
                        }
                        onStatusChanged: {
                            if (status === Loader.Loading || status === Loader.Null) {
                                root.startupModelSettled = false
                            }
                        }
                    }
                    Connections {
                        target: carViewLoader.item
                        ignoreUnknownSignals: true

                        function onModelReadyChanged() {
                            root.startupModelSettled = carViewLoader.item !== null && carViewLoader.item.modelReady
                            if (root.startupModelSettled)
                                root.handleStartupReadinessChanged()
                        }
                    }
                    Binding {
                        target: carViewLoader.item
                        property: "gearMode"
                        value: root.gear_mode
                        when: carViewLoader.item !== null
                    }
                    Binding {
                        target: carViewLoader.item
                        property: "enable3DAnimation"
                        value: root.enable3DAnimation
                        when: carViewLoader.item !== null
                    }
                    Binding {
                        target: carViewLoader.item
                        property: "highQuality"
                        value: root.useHighQuality3D
                        when: carViewLoader.item !== null
                    }
                    Binding {
                        target: carViewLoader.item
                        property: "cameraTransitionDuration"
                        value: root.cameraTransitionDuration
                        when: carViewLoader.item !== null
                    }
                    Binding {
                        target: carViewLoader.item
                        property: "interactive"
                        value: root.gear_mode === "P" || root.gear_mode === "N"
                        when: carViewLoader.item !== null
                    }
                    Binding {
                        target: carViewLoader.item
                        property: "vehicleSpeed"
                        value: rightGauge.speed
                        when: carViewLoader.item !== null
                    }
                }

                // Image{
                //     id: speedLimitSign
                //     z: 2
                //     source: "qrc:/asset/icons/Road/ss.svg"
                //     sourceSize: Qt.size(root.s(60), root.s(76))
                //     scale: root.startupScale(3.45)
                //     anchors.top: topBar.bottom
                //     anchors.horizontalCenter: topBar.horizontalCenter
                //     anchors.horizontalCenterOffset: -root.s(180)
                //     anchors.topMargin: root.s(8)
                //     opacity: root.startupOpacity(3.45)
                //     Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                //     Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }

                //     Text{
                //         anchors.centerIn: parent
                //         anchors.verticalCenterOffset: root.s(15)
                //         font.pixelSize: root.s(28)
                //         font.bold: true
                //         font.weight: Font.Normal
                //         font.family: root.textFontFamily
                //         color: "#090C14"
                //         text: qsTr("90")
                //     }
                // }

                RowLayout {
                    id: tripInfo
                    width: modelCar.width * 0.52
                    height: root.s(34)

                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.verticalCenterOffset: root.s(170)

                    opacity: root.startupOpacity(3.65)
                    scale: root.startupScale(3.65)

                    spacing: 0

                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }

                    RowLayout {
                        Layout.preferredWidth: tripInfo.width / 2
                        Layout.fillHeight: true
                        spacing: root.s(6)

                        Image {
                            Layout.alignment: Qt.AlignVCenter
                            source: "qrc:/asset/icons/Road/mdi_map-marker-outline.svg"
                            sourceSize: Qt.size(root.s(24), root.s(24))
                        }

                        Label {
                            id: remainingDistanceLabel
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: root.s(112)

                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter

                            font.pixelSize: root.s(20)
                            font.bold: true
                            font.weight: Font.Normal
                            font.family: root.textFontFamily
                            text: root.formatRouteDistance(clusterMapBackground.routeRemainingMeters,
                                                        clusterMapBackground.routeTotalMeters)
                            color: "#FFFFFF"
                        }
                    }

                    RowLayout {
                        Layout.preferredWidth: tripInfo.width / 2
                        Layout.fillHeight: true
                        spacing: root.s(6)
                        layoutDirection: Qt.RightToLeft

                        Image {
                            Layout.alignment: Qt.AlignVCenter
                            source: "qrc:/asset/icons/Road/mdi_clock-time-four-outline.svg"
                            sourceSize: Qt.size(root.s(24), root.s(24))
                        }

                        Label {
                            id: remainingTimeLabel
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: root.s(112)

                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter

                            font.pixelSize: root.s(20)
                            font.bold: true
                            font.weight: Font.Normal
                            font.family: root.textFontFamily
                            text: root.formatRouteTime(clusterMapBackground.routeRemainingSeconds,
                                                    clusterMapBackground.routeTotalMeters)
                            color: "#FFFFFF"
                        }
                    }
                }
                Image{
                    id:rightgaugae
                    sourceSize: Qt.size(root.gaugeSize - 29, root.gaugeSize - 29)
                    anchors.right: parent.right
                    anchors.rightMargin: root.gaugeSideMargin + 15
                    anchors.verticalCenterOffset: root.gaugeYOffset + 10
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/asset/Speedometer.png"
                    opacity: root.startupOpacity(2.15)
                    scale: root.startupScale(2.15)
                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    CircularGaugeMeter {
                        id:rightGauge
                        z: -1
                        anchors.centerIn: parent
                        property bool accelerating
                        property int speed: 0
                        anchors.verticalCenterOffset: - 10
                        width: (parent.width + 29)* 0.67
                        height: (parent.height+ 29) * 0.67
                        value: speed//accelerating ? maximumValue : 0
                        maximumValue: 220
                        shadowVisible: true
                        Behavior on value { NumberAnimation { duration: 300 }}
                        //NumberAnimation on speed { duration: 5000; easing.type: Easing.InOutQuad; from: 0; to: 240; loops: Animation.Infinite; running: root.demoMode }
                        Text{
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: -root.s(24)
                            font.bold: Font.DemiBold
                            font.weight: Font.Normal
                            font.pixelSize: root.s(38)
                            color: "#FFFFFF"
                            font.family: root.textFontFamily
                            text: rightGauge.speed//rightGuage.value.toFixed(0)
                        }

                        Text{
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: root.s(22)
                            font.pixelSize: root.s(13)
                            font.bold: Font.DemiBold
                            font.weight: Font.Normal
                            color: "#FFFFFF"
                            font.family: root.textFontFamily
                            text: "Km/h"
                        }
                    }
                }

                Image{
                    id: fuelIcon
                    source: "qrc:/asset/icons/feaul.svg"
                    anchors.bottom: left.top
                    anchors.left: left.left
                    sourceSize: Qt.size(root.s(34), root.s(34))
                    anchors.bottomMargin: root.s(4)
                    opacity: root.startupOpacity(2.55)
                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                }



                FuelShader{
                    id:left
                    width: root.fuelArcWidth
                    height: root.fuelArcHeight
                    anchors.left: leftgauge.left
                    anchors.bottom: leftgauge.bottom
                    anchors.leftMargin: root.s(6)
                    anchors.bottomMargin: root.s(46)
                    opacity: root.startupOpacity(2.55)
                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    // NumberAnimation on fuelLevel {
                    //     duration: 5000
                    //     from: 100
                    //     to: 0
                    //     easing.type: Easing.InOutQuad
                    //     loops: Animation.Infinite
                    //     running: root.demoMode
                    // }
                    Behavior on fuelLevel {
                        NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
                    }
                }


                Image{
                    id: coolantIcon
                    source: "qrc:/asset/icons/desal.svg"
                    anchors.bottom: right.top
                    anchors.right: right.right
                    sourceSize: Qt.size(root.s(34), root.s(34))
                    anchors.bottomMargin: root.s(4)
                    opacity: root.startupOpacity(2.65)
                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                }
                CoolantShader{
                    id:right
                    width: root.fuelArcWidth
                    height: root.fuelArcHeight
                    anchors.left: rightgaugae.left
                    anchors.leftMargin: rightgaugae.width /2
                    anchors.bottom: rightgaugae.bottom
                    anchors.bottomMargin: root.s(32)
                    opacity: root.startupOpacity(2.65)
                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    // NumberAnimation on fuelLevel {
                    //     duration: 5000
                    //     from: 100
                    //     to: 0
                    //     easing.type: Easing.InOutQuad
                    //     loops: Animation.Infinite
                    //     running: root.demoMode
                    // }
                    Behavior on fuelLevel {
                        NumberAnimation { duration: 100; easing.type: Easing.InOutQuad }
                    }
                }

                Image{
                    id: bottomGearBar
                    sourceSize: Qt.size(root.bottomBarWidth, root.bottomBarHeight)
                    source: "qrc:/asset/icons/bottom.png"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    opacity: root.startupOpacity(5.15)
                    scale: root.startupScale(5.15)
                    Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    Behavior on scale { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }
                    Component.onCompleted: gearP.checked = true

                    RowLayout{
                        spacing: root.gearSpacing
                        anchors.right: middle.left
                        anchors.rightMargin: root.gearOuterGap
                        anchors.verticalCenter: middle.verticalCenter
                        IconButton{
                            id: gearP
                            roundIcon: true
                            allowUncheck: false
                            ButtonGroup.group: gearGroup
                            Layout.preferredWidth: root.gearButtonSize
                            Layout.preferredHeight: root.gearButtonSize
                            iconWidth: root.gearButtonSize
                            iconHeight: root.gearButtonSize
                            font.pixelSize: root.s(26)
                            font.bold: true
                            font.weight: Font.Normal
                            font.family: root.textFontFamily
                            text: "P"
                            checkable: true
                            setIconColor: checked ? "#A1A1A1" : "white"
                            scale: checked ? 1.6:1
                            Behavior on scale{NumberAnimation{duration: 200}}
                            onCheckedChanged:{
                                if (!checked)
                                    return
                                switch(gear_mode){
                                    case "R":
                                        gearR.checked = false
                                        break
                                    case "D":
                                        gearD.checked = false
                                        break
                                    case "N":
                                        gearN.checked = false
                                        break
                                }
                                root.gear_mode = "P"
                                root.showCarFrontView()
                                root.moveGearIndicator(gearP)
                            }
                        }
                        IconButton{
                            id: gearN
                            roundIcon: true
                            allowUncheck: false
                            ButtonGroup.group: gearGroup
                            Layout.preferredWidth: root.gearButtonSize
                            Layout.preferredHeight: root.gearButtonSize
                            iconWidth: root.gearButtonSize
                            iconHeight: root.gearButtonSize
                            font.pixelSize: root.s(26)
                            font.bold: true
                            font.weight: Font.Normal
                            font.family: root.textFontFamily
                            text: "N"
                            setIconColor: checked ? "#A1A1A1" : "white"
                            checkable: true
                            scale: checked ? 1.6:1
                            Behavior on scale{NumberAnimation{duration: 200}}
                            onCheckedChanged:{
                                if (!checked)
                                    return
                                switch(gear_mode){
                                    case "R":
                                        gearR.checked = false
                                        break
                                    case "P":
                                        gearP.checked = false
                                        break
                                    case "D":
                                        gearD.checked = false
                                        break
                                }
                                root.gear_mode = "N"
                                root.showCarFrontView()
                                root.moveGearIndicator(gearN)
                            }
                        }
                    }
                    Item{
                        id: middle
                        width: root.s(150)
                        height: parent.height
                        anchors.centerIn: parent
                        Text{
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: -root.s(18)
                            font.bold: Font.DemiBold
                            font.weight: Font.Normal
                            font.pixelSize: root.s(28)
                            color: "#FFFFFF"
                            font.family: root.textFontFamily
                            text: "ODO"//rightGuage.value.toFixed(0)
                        }

                        Text{
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: root.s(18)
                            Layout.alignment: Qt.AlignVCenter
                            font.bold: Font.DemiBold
                            font.weight: Font.Normal
                            font.pixelSize: root.s(13)
                            color: "#FFFFFF"
                            font.family: root.textFontFamily
                            text: "1000 km"
                        }
                        Image{
                            id: line_devide_odo
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.verticalCenterOffset: root.s(39)
                            width: root.s(110)
                            height: root.s(3)
                            source: "qrc:/asset/icons/line_devider.png"
                            visible: true
                        }
                    }

                    RowLayout{
                        spacing: root.gearSpacing
                        anchors.left: middle.right
                        anchors.leftMargin: root.gearOuterGap
                        anchors.verticalCenter: middle.verticalCenter
                        IconButton{
                            id: gearD
                            roundIcon: true
                            allowUncheck: false
                            ButtonGroup.group: gearGroup
                            Layout.preferredWidth: root.gearButtonSize
                            Layout.preferredHeight: root.gearButtonSize
                            iconWidth: root.gearButtonSize
                            iconHeight: root.gearButtonSize
                            font.pixelSize: root.s(26)
                            font.bold: true
                            font.weight: Font.Normal
                            font.family: root.textFontFamily
                            text: "D"
                            setIconColor: checked ? "#A1A1A1" : "white"
                            checkable: true
                            scale: checked ? 1.6:1
                            Behavior on scale{NumberAnimation{duration:200}}
                            onCheckedChanged:{
                                if (!checked)
                                    return
                                switch(gear_mode){
                                    case "R":
                                        gearR.checked = false
                                        break
                                    case "P":
                                        gearP.checked = false
                                        break
                                    case "N":
                                        gearN.checked = false
                                        break
                                }
                                root.gear_mode = "D"
                                root.showCarRearView()
                                root.moveGearIndicator(gearD)
                            }
                        }
                        IconButton{
                            id: gearR
                            roundIcon: true
                            allowUncheck: false
                            ButtonGroup.group: gearGroup
                            Layout.preferredWidth: root.gearButtonSize
                            Layout.preferredHeight: root.gearButtonSize
                            iconWidth: root.gearButtonSize
                            iconHeight: root.gearButtonSize
                            font.pixelSize: root.s(26)
                            font.bold: true
                            font.weight: Font.Normal
                            font.family: root.textFontFamily
                            text: "R"
                            setIconColor: checked ? "#A1A1A1" : "white"
                            checkable: true
                            scale: checked ? 1.6:1
                            Behavior on scale{NumberAnimation{duration: 200}}
                            onCheckedChanged:{
                                if (!checked)
                                    return
                                switch(gear_mode){
                                    case "N":
                                        gearN.checked = false
                                        break
                                    case "P":
                                        gearP.checked = false
                                        break
                                    case "D":
                                        gearD.checked = false
                                        break
                                }
                                root.gear_mode = "R"
                                root.showCarRearView()
                                root.moveGearIndicator(gearR)
                            }
                        }
                    }

                    Image{
                        id: line_devide
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.verticalCenterOffset: root.s(31)
                        x: 0
                        width: root.s(70)
                        height: root.s(3)
                        source: "qrc:/asset/icons/line_devider.png"
                        visible: false
                        Behavior on x {NumberAnimation{duration: 200}}
                    }

                    Connections{
                        target: Cluster
                        // function onSignal_light_high(mode){
                        //     lightHigh.checked = mode
                        // }
                        // function onSignal_light_dim(mode){
                        //     lightDimmed.checked = mode
                        // }

                        // function onSignal_fog(mode){
                        //     lightFog.checked = mode
                        // }

                        // function onSignal_light_right(mode){
                        //     rightIndicator.checked = mode
                        // }

                        // function onSignal_light_left(mode){
                        //     leftIndicator.checked = mode
                        // }

                        // function onSignal_hazard(mode){
                        //     rightIndicator1.checked = mode
                        //     rightIndicator1.visible = mode
                        //     leftIndicator1.checked = mode
                        //     leftIndicator1.visible = mode
                        //     rightIndicator.visible = !mode
                        //     leftIndicator.visible = !mode
                        // }

                        function onSpeed(value){
                            root.switchToSerialMode()
                            var speedKph = Number(value)
                            if (!isFinite(speedKph))
                                speedKph = 0
                            speedKph = Math.max(0, speedKph)
                            root.mapDriveSpeedKph = speedKph
                            rightGauge.speed = speedKph
                            leftGauge.speed = speedKph
                            if (root.gear_mode === "D" && speedKph <= clusterMapBackground.stoppedSpeedThresholdKph)
                                clusterMapBackground.pushDriveSpeed()
                            root.traceSerialUi("speed", speedKph)
                        }
                        function onCoolant(value){
                            root.switchToSerialMode()
                            right.fuelLevel = value
                            root.traceSerialUi("coolant", value)
                        }
                        function onFuel(value){
                            root.switchToSerialMode()
                            left.fuelLevel = value
                            root.traceSerialUi("fuel", value)
                        }

                        // function onGear(gear){
                        //     switch(gear){
                        //         case "P":
                        //             gearP.checked = true
                        //             break
                        //         case "N":
                        //             gearN.checked = true
                        //             break
                        //         case "D":
                        //             gearD.checked = true
                        //             break
                        //         case "R":
                        //             gearR.checked = true
                        //             break
                        //     }
                        // }
                    }
                    }
                }

            Rectangle {
                id: arrivalPanel
                z: 50
                width: root.s(560)
                height: root.s(138)
                anchors.centerIn: parent
                radius: root.s(8)
                color: "#0B1220"
                border.color: "#70D7FF"
                border.width: root.s(1)
                visible: root.mapArrivalPanelVisible || opacity > 0.01
                opacity: root.mapArrivalPanelVisible ? 0.94 : 0

                Behavior on opacity { NumberAnimation { duration: 180; easing.type: Easing.InOutQuad } }

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: root.s(8)
                    radius: root.s(6)
                    color: "transparent"
                    border.color: "#1f3f62"
                    border.width: root.s(1)
                }

                Text {
                    anchors.centerIn: parent
                    width: parent.width - root.s(64)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    font.pixelSize: root.s(24)
                    font.bold: true
                    font.family: root.textFontFamily
                    color: "#FFFFFF"
                    text: root.mapArrivalText
                }
            }

            MouseArea {
                id: rootInputTrace
                anchors.fill: parent
                z: -0.5
                acceptedButtons: Qt.AllButtons
                propagateComposedEvents: true

                onPressed: function(mouse) {
                    root.traceInput("pressed", mouse.x, mouse.y)
                    mouse.accepted = false
                }

                onReleased: function(mouse) {
                    root.traceInput("released", mouse.x, mouse.y)
                    mouse.accepted = false
                }

                onClicked: function(mouse) {
                    root.traceInput("clicked", mouse.x, mouse.y)
                    mouse.accepted = false
                }
            }
        }
    }
}
