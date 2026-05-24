import QtQuick
import QtWebEngine
import QtPositioning

Item {
    id: surface

    property var centerCoordinate
    property real zoomLevel: 14
    property real pitch: 52
    property real bearing: 0
    property bool map3dEnabled: true
    property bool followExternalCenter: true
    property var driveStartCoordinate
    property var driveEndCoordinate
    property var routePath: []
    property real driveSpeedKph: 0
    property real stoppedSpeedThresholdKph: 0.5
    property bool driveAnimationEnabled: true
    property int navigationSessionId: 0
    property real firstPersonZoom: 16//18.25
    property real firstPersonPitch: 50//74

    property string rendererUrl: "qrc:/goong-map.html"
    property string goongMapTilesKey: "Xgw2Eiqb38KmKuSsxQFH5c4NtEuORfbAdWfbxXIB"
    property string goongStyleUrl: "https://tiles.goong.io/assets/goong_map_web.json"
    property string vectorTileUrl: "http://127.0.0.1:8080/data/v3.json"
    property string buildingSourceLayer: "VN_Building"

    property bool ready: false
    property bool receivingCameraFromWeb: false
    property bool pendingFocusMapCenter: false
    property bool pendingDriveAnimation: false
    property bool driveSessionActive: false
    property real routeProgressMeters: 0
    property real routeTotalMeters: 0
    property real routeRemainingMeters: 0
    property real routeRemainingSeconds: -1
    property bool navigationResetActive: false

    property var map: mapObject

    signal userGestureStarted()

    Component.onCompleted: {
        console.warn("CLUSTER_MAP_COMPONENT_READY",
                     "size", width + "x" + height,
                     "url", rendererUrl,
                     "hasKey", goongMapTilesKey.length > 0)

        if (validCoord(centerCoordinate) && (!validCoord(map.center) || followExternalCenter))
            map.center = centerCoordinate

        if (isFinite(zoomLevel))
            map.zoomLevel = zoomLevel

        if (isFinite(pitch))
            map.tilt = pitch

        if (isFinite(bearing))
            map.bearing = bearing

        scheduleSync()
    }

    QtObject {
        id: mapObject
        property var center: null
        property real zoomLevel: 14
        property real tilt: 52
        property real bearing: 0
        readonly property real minimumZoomLevel: 2
        readonly property real maximumZoomLevel: 20
        readonly property real minimumTilt: 0
        readonly property real maximumTilt: 80

        onCenterChanged: surface.scheduleSync()
        onZoomLevelChanged: surface.scheduleSync()
        onTiltChanged: surface.scheduleSync()
        onBearingChanged: surface.scheduleSync()
    }

    function validCoord(coord) {
        return coord
                && isFinite(coord.latitude) && isFinite(coord.longitude)
                && coord.latitude >= -90 && coord.latitude <= 90
                && coord.longitude >= -180 && coord.longitude <= 180
    }

    function coordPayload(coord) {
        if (!validCoord(coord))
            return null

        return {
            "lat": coord.latitude,
            "lon": coord.longitude
        }
    }

    function routePayload(path) {
        var line = []
        if (!path)
            return line

        for (var i = 0; i < path.length; ++i) {
            var point = path[i]
            if (validCoord(point))
                line.push([point.longitude, point.latitude])
        }

        return line
    }

    function panBy(dx, dy) {
        if (!validCoord(map.center))
            map.center = centerCoordinate
        if (!validCoord(map.center))
            return

        var center = map.center
        var zoom = Math.max(0.1, map.zoomLevel)
        var latRad = center.latitude * Math.PI / 180.0
        var earthRadius = 6378137.0
        var metersPerPixel = (2 * Math.PI * earthRadius * Math.cos(latRad)) / (256 * Math.pow(2, zoom))
        var latDelta = (dy * metersPerPixel / earthRadius) * 180.0 / Math.PI
        var lonScale = Math.max(0.25, Math.cos(latRad))
        var lonDelta = (dx * metersPerPixel / (earthRadius * lonScale)) * 180.0 / Math.PI

        map.center = QtPositioning.coordinate(center.latitude + latDelta,
                                              center.longitude - lonDelta)
    }

    function zoomBy(delta) {
        map.zoomLevel = Math.max(map.minimumZoomLevel,
                                 Math.min(map.maximumZoomLevel, map.zoomLevel + delta))
    }

    function rotateBy(delta) {
        map.bearing = map.bearing + delta
    }

    function tiltBy(delta) {
        map.tilt = Math.max(map.minimumTilt,
                            Math.min(map.maximumTilt, map.tilt + delta))
    }

    function routeHasPath() {
        return routePath && routePath.length > 1
    }

    function scheduleSync() {
        if (!ready || receivingCameraFromWeb || navigationResetActive)
            return

        if (!driveSessionActive && !routeHasPath())
            return

        syncTimer.restart()
    }

    function applyWebCamera(camera) {
        if (!camera)
            return

        if (navigationResetActive || driveSessionActive || (!driveSessionActive && !routeHasPath())) {
            console.warn("MAPLIBRE_CAMERA_IGNORED_RESET_OR_ACTIVE_DRIVE")
            return
        }

        receivingCameraFromWeb = true

        if (isFinite(camera.lat) && isFinite(camera.lon))
            map.center = QtPositioning.coordinate(camera.lat, camera.lon)

        if (isFinite(camera.zoom))
            map.zoomLevel = Math.max(map.minimumZoomLevel,
                                     Math.min(map.maximumZoomLevel, camera.zoom))

        if (isFinite(camera.pitch))
            map.tilt = Math.max(map.minimumTilt,
                                Math.min(map.maximumTilt, camera.pitch))

        if (isFinite(camera.bearing))
            map.bearing = camera.bearing

        receivingCameraFromWeb = false
    }

    function applyRouteProgress(progress) {
        if (!progress)
            return

        if (isFinite(progress.progressMeters))
            routeProgressMeters = Math.max(0, progress.progressMeters)
        if (isFinite(progress.totalMeters))
            routeTotalMeters = Math.max(0, progress.totalMeters)
        if (isFinite(progress.remainingMeters))
            routeRemainingMeters = Math.max(0, progress.remainingMeters)
        if (isFinite(progress.remainingSeconds))
            routeRemainingSeconds = progress.remainingSeconds
        else
            routeRemainingSeconds = -1
    }

    function syncPayload(center, instant) {
        return {
            "center": coordPayload(center || map.center || centerCoordinate),
            "zoom": map.zoomLevel,
            "pitch": map3dEnabled ? map.tilt : 0,
            "bearing": map.bearing,
            "enabled3d": map3dEnabled,
            "instant": !!instant,
            "driveStart": coordPayload(driveStartCoordinate || centerCoordinate),
            "driveEnd": coordPayload(driveEndCoordinate),
            "route": routePayload(routePath),
            "driveActive": driveSessionActive,
            "driveSpeedKph": Math.max(0, driveSpeedKph),
            "stoppedSpeedThresholdKph": Math.max(0, stoppedSpeedThresholdKph),
            "driveAnimationEnabled": driveAnimationEnabled,
            "navigationSessionId": navigationSessionId,
            "firstPersonZoom": firstPersonZoom,
            "firstPersonPitch": firstPersonPitch,
            "vectorTileUrl": vectorTileUrl,
            "buildingSourceLayer": buildingSourceLayer,
            "goongMapTilesKey": goongMapTilesKey,
            "goongStyleUrl": goongStyleUrl
        }
    }

    function sync() {
        if (!ready)
            return

        var payload = syncPayload(map.center || centerCoordinate, false)

        web.runJavaScript("window.navigationMap && window.navigationMap.update("
                          + JSON.stringify(payload) + ");")
    }

    function focusMapCenter(startDriveAnimation) {
        var useDriveCamera = !!startDriveAnimation && routeHasPath()
        var center = useDriveCamera && validCoord(driveStartCoordinate)
                ? driveStartCoordinate
                : (validCoord(centerCoordinate) ? centerCoordinate : map.center)
        if (validCoord(center))
            map.center = center

        if (useDriveCamera && isFinite(firstPersonZoom))
            map.zoomLevel = firstPersonZoom
        else if (isFinite(zoomLevel))
            map.zoomLevel = zoomLevel
        if (useDriveCamera && isFinite(firstPersonPitch))
            map.tilt = firstPersonPitch
        else if (isFinite(pitch))
            map.tilt = pitch
        if (isFinite(bearing))
            map.bearing = bearing

        if (!ready) {
            pendingFocusMapCenter = true
            pendingDriveAnimation = useDriveCamera
            return
        }

        pendingFocusMapCenter = false
        pendingDriveAnimation = false
        syncTimer.stop()

        if (navigationResetActive)
            return

        var payload = syncPayload(center, true)
        payload.startDriveAnimation = useDriveCamera
        web.runJavaScript("window.navigationMap && window.navigationMap.update("
                          + JSON.stringify(payload) + ");")
    }

    function activateFirstPersonDrive() {
        if (!routeHasPath()) {
            console.warn("CLUSTER_MAP_ACTIVATE_IGNORED_NO_ROUTE")
            return
        }

        navigationResetActive = false
        driveSessionActive = true
        focusMapCenter(true)
    }

    function stopDriveAnimation() {
        pendingDriveAnimation = false
        driveSessionActive = false
        syncTimer.stop()
        if (!ready)
            return

        web.runJavaScript("window.navigationMap && window.navigationMap.stopDriveAnimation && window.navigationMap.stopDriveAnimation();")
    }

    function resetNavigationState() {
        pendingFocusMapCenter = false
        pendingDriveAnimation = false
        driveSessionActive = false
        navigationResetActive = true

        routeProgressMeters = 0
        routeTotalMeters = 0
        routeRemainingMeters = 0
        routeRemainingSeconds = -1

        syncTimer.stop()
        navigationResetReleaseTimer.restart()

        if (!ready)
            return

        var clearedSessionId = navigationSessionId > 0 ? navigationSessionId - 1 : -1
        web.runJavaScript("window.navigationMap && window.navigationMap.clearNavigationState && window.navigationMap.clearNavigationState("
                        + JSON.stringify(clearedSessionId) + ");")
    }

    function pushDriveSpeed() {
        if (!ready)
            return

        web.runJavaScript("window.navigationMap && window.navigationMap.setDriveSpeed && window.navigationMap.setDriveSpeed("
                          + JSON.stringify(Math.max(0, driveSpeedKph)) + ", "
                          + JSON.stringify(Math.max(0, stoppedSpeedThresholdKph)) + ");")
    }

    function resetStyle() {
        web.runJavaScript("window.navigationMap && window.navigationMap.resetStyle && window.navigationMap.resetStyle("
                          + JSON.stringify(vectorTileUrl) + ", "
                          + JSON.stringify(buildingSourceLayer) + ", "
                          + JSON.stringify(goongMapTilesKey) + ", "
                          + JSON.stringify(goongStyleUrl) + ");")
    }

    function sameCoord(a, b) {
        return validCoord(a) && validCoord(b)
                && a.latitude === b.latitude
                && a.longitude === b.longitude
    }

    onCenterCoordinateChanged: {
        if (followExternalCenter && !sameCoord(map.center, centerCoordinate))
            map.center = centerCoordinate
        scheduleSync()
    }

    onZoomLevelChanged: {
        if (map.zoomLevel !== zoomLevel)
            map.zoomLevel = zoomLevel
        scheduleSync()
    }

    onPitchChanged: {
        if (map.tilt !== pitch)
            map.tilt = pitch
        scheduleSync()
    }

    onBearingChanged: {
        if (map.bearing !== bearing)
            map.bearing = bearing
        scheduleSync()
    }

    onMap3dEnabledChanged: scheduleSync()
    onDriveStartCoordinateChanged: scheduleSync()
    onDriveEndCoordinateChanged: scheduleSync()
    onRoutePathChanged: {
        routeProgressMeters = 0
        routeTotalMeters = 0
        routeRemainingMeters = 0
        routeRemainingSeconds = -1

        if (!routeHasPath()) {
            resetNavigationState()
            return
        }

        navigationResetActive = false
        scheduleSync()
    }
    onDriveSpeedKphChanged: {
        pushDriveSpeed()

        if (driveSpeedKph > stoppedSpeedThresholdKph && driveSessionActive && routeHasPath())
            scheduleSync()
    }
    onStoppedSpeedThresholdKphChanged: {
        pushDriveSpeed()

        if (driveSpeedKph > stoppedSpeedThresholdKph && driveSessionActive && routeHasPath())
            scheduleSync()
    }
    onDriveAnimationEnabledChanged: scheduleSync()
    onNavigationSessionIdChanged: {
        if (!routeHasPath())
            resetNavigationState()
        else
            scheduleSync()
    }
    onFirstPersonZoomChanged: scheduleSync()
    onFirstPersonPitchChanged: scheduleSync()
    onVectorTileUrlChanged: scheduleSync()
    onBuildingSourceLayerChanged: scheduleSync()
    onGoongMapTilesKeyChanged: scheduleSync()
    onGoongStyleUrlChanged: scheduleSync()

    Timer {
        id: syncTimer
        interval: 40
        repeat: false
        onTriggered: surface.sync()
    }

    Timer {
        id: navigationResetReleaseTimer
        interval: 650
        repeat: false
        onTriggered: surface.navigationResetActive = false
    }

    WebEngineProfile {
        id: mapProfile

        httpUserAgent: "Mozilla/5.0 (X11; Linux aarch64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
    }

    WebEngineView {
        id: web
        anchors.fill: parent
        profile: mapProfile
        // Source map cu OpenMapTiles local van giu lai trong qml/maplibre.html,
        // nhung khong su dung nua:
        // url: "qrc:/maplibre.html"
        url: surface.rendererUrl
        backgroundColor: "#0b1220"

        settings.javascriptEnabled: true
        settings.localContentCanAccessFileUrls: true
        settings.localContentCanAccessRemoteUrls: true
        settings.webGLEnabled: true

        onLoadingChanged: function(loadRequest) {
            if (loadRequest.status === WebEngineView.LoadSucceededStatus) {
                console.warn("MAPLIBRE_WEBENGINE_LOAD_OK")
                surface.ready = true
                if (surface.pendingFocusMapCenter)
                    surface.focusMapCenter(surface.pendingDriveAnimation)
                else
                    surface.sync()
            } else if (loadRequest.status === WebEngineView.LoadFailedStatus) {
                console.warn("MAPLIBRE_WEBENGINE_LOAD_FAILED:",
                             loadRequest.errorCode,
                             loadRequest.errorString)
            }
        }

        onJavaScriptConsoleMessage: function(level, message, lineNumber, sourceID) {
            if (message === "MAPLIBRE_USER_GESTURE_START") {
                surface.userGestureStarted()
                return
            }

            if (String(message).indexOf("MAPLIBRE_CAMERA=") === 0) {
                try {
                    surface.applyWebCamera(JSON.parse(String(message).substring(15)))
                } catch (e) {
                    console.warn("MAPLIBRE_CAMERA_PARSE_FAILED:", e)
                }
                return
            }

            var routeProgressPrefix = "MAPLIBRE_ROUTE_PROGRESS="
            if (String(message).indexOf(routeProgressPrefix) === 0) {
                try {
                    surface.applyRouteProgress(JSON.parse(String(message).substring(routeProgressPrefix.length)))
                } catch (e) {
                    console.warn("MAPLIBRE_ROUTE_PROGRESS_PARSE_FAILED:", e)
                }
                return
            }

            console.warn("WEBENGINE_CONSOLE:", level, message, sourceID, lineNumber)
        }
    }
}
