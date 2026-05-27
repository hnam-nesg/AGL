import QtQuick
import QtPositioning
import QtWebEngine

Item {
    id: surface

    property var center
    property var currentCoordinate
    property var destinationCoordinate
    property var routePath: []
    property bool navigationActive: false
    property real zoomLevel: 20
    property real tilt: 80
    property real bearing: 0
    property bool map3dEnabled: true

    property string rendererUrl: "qrc:/home-map-3d.html"
    property string fallbackGoongMapTilesKey: "Xgw2Eiqb38KmKuSsxQFH5c4NtEuORfbAdWfbxXIB"
    property string goongMapTilesKey: fallbackGoongMapTilesKey
    property string goongStyleUrl: "https://tiles.goong.io/assets/goong_map_web.json"
    property string buildingSourceLayer: "VN_Building"

    readonly property real minimumZoomLevel: 2
    readonly property real maximumZoomLevel: 20
    readonly property real minimumTilt: 0
    readonly property real maximumTilt: 80

    property bool ready: false
    property bool loadFailed: false
    property bool loadComplete: ready || loadFailed
    property bool receivingCameraFromWeb: false
    property real driveSpeedKph: 0
    property real stoppedSpeedThresholdKph: 0.5
    property bool driveAnimationEnabled: true
    property bool driveSessionActive: navigationActive
    property real firstPersonOffsetY: 56
    property int routeSessionSeed: 0
    property int navigationSessionId: 0
    property string driveRouteKey: ""
    property int driveRouteSessionSeed: -1
    property real routeProgressMeters: 0
    property real routeTotalMeters: 0
    property real routeRemainingMeters: 0
    property real routeRemainingSeconds: -1
    property bool driveRouteArrived: false
    property int routeResetSeed: 0
    property int appliedRouteResetSeed: 0

    signal userGestureStarted()
    signal driveArrived(var progress)

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

    function routeHasPath() {
        return routePayload(routePath).length > 1
    }

    function routeKey(path) {
        var line = routePayload(path)
        if (line.length <= 1)
            return ""

        var first = line[0]
        var last = line[line.length - 1]
        return line.length + ":"
                + first[0].toFixed(6) + "," + first[1].toFixed(6) + ":"
                + last[0].toFixed(6) + "," + last[1].toFixed(6)
    }

    function clearDriveState() {
        driveRouteKey = ""
        driveRouteSessionSeed = routeSessionSeed
        driveSpeedKph = 0
        routeProgressMeters = 0
        routeTotalMeters = 0
        routeRemainingMeters = 0
        routeRemainingSeconds = -1
        driveRouteArrived = false
    }

    function clearRendererNavigationState() {
        if (!ready)
            return

        var clearedSessionId = navigationSessionId
        web.runJavaScript("window.homeMiniMap && window.homeMiniMap.clearNavigationState && window.homeMiniMap.clearNavigationState("
                          + JSON.stringify(clearedSessionId) + ");")
        navigationSessionId = clearedSessionId + 1
    }

    function applyRouteResetIfNeeded() {
        if (routeResetSeed === appliedRouteResetSeed)
            return

        appliedRouteResetSeed = routeResetSeed
        navigationSessionId += 1
        clearDriveState()
        clearRendererNavigationState()
    }

    function updateDriveSession() {
        var key = routeKey(routePath)
        if (!navigationActive || key.length === 0) {
            var hadActiveDrive = driveRouteKey.length > 0
                    || driveSpeedKph > 0
                    || routeTotalMeters > 0
                    || routeProgressMeters > 0
            if (hadActiveDrive) {
                navigationSessionId += 1
                clearDriveState()
                clearRendererNavigationState()
            } else {
                clearDriveState()
            }
            return
        }

        if (key !== driveRouteKey || routeSessionSeed !== driveRouteSessionSeed) {
            driveRouteKey = key
            driveRouteSessionSeed = routeSessionSeed
            navigationSessionId += 1
            driveSpeedKph = driveSessionActive ? 150 : 0
            routeProgressMeters = 0
            routeTotalMeters = 0
            routeRemainingMeters = 0
            routeRemainingSeconds = -1
            driveRouteArrived = false
        }
    }

    function scheduleSync() {
        if (!ready || receivingCameraFromWeb)
            return

        syncTimer.restart()
    }

    function applyWebCamera(camera) {
        if (!camera)
            return

        receivingCameraFromWeb = true

        if (isFinite(camera.lat) && isFinite(camera.lon))
            center = QtPositioning.coordinate(camera.lat, camera.lon)

        if (isFinite(camera.zoom))
            zoomLevel = Math.max(minimumZoomLevel, Math.min(maximumZoomLevel, camera.zoom))

        if (isFinite(camera.pitch))
            tilt = Math.max(minimumTilt, Math.min(maximumTilt, camera.pitch))

        if (isFinite(camera.bearing))
            bearing = camera.bearing

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

        var arrived = routeTotalMeters > 0
                && (routeRemainingMeters <= 3 || routeProgressMeters >= routeTotalMeters - 1)
        if (arrived) {
            driveSpeedKph = 0
            if (!driveRouteArrived) {
                driveRouteArrived = true
                driveArrived(progress)
            }
        }
    }

    function pushDriveSpeed() {
        if (!ready)
            return

        web.runJavaScript("window.homeMiniMap && window.homeMiniMap.setDriveSpeed && window.homeMiniMap.setDriveSpeed("
                          + JSON.stringify(Math.max(0, driveSpeedKph)) + ", "
                          + JSON.stringify(Math.max(0, stoppedSpeedThresholdKph)) + ");")
    }

    function sync() {
        if (!ready)
            return

        updateDriveSession()
        applyRouteResetIfNeeded()
        var routeLine = routePayload(routePath)
        var routeVisible = navigationActive
        var canDrive = navigationActive && driveSessionActive && routeLine.length > 1
        var payload = {
            "center": coordPayload(center),
            "current": coordPayload(currentCoordinate),
            "destination": coordPayload(destinationCoordinate),
            "route": routeLine,
            "navigationActive": routeVisible,
            "driveActive": canDrive,
            "driveSpeedKph": Math.max(0, driveSpeedKph),
            "stoppedSpeedThresholdKph": Math.max(0, stoppedSpeedThresholdKph),
            "driveAnimationEnabled": canDrive && driveAnimationEnabled,
            "navigationSessionId": navigationSessionId,
            "firstPersonZoom": Math.max(17.2, Math.min(20.0, zoomLevel)),
            "firstPersonPitch": Math.max(60.0, Math.min(79.0, tilt)),
            "firstPersonOffsetY": firstPersonOffsetY,
            "zoom": zoomLevel,
            "pitch": map3dEnabled ? tilt : 0,
            "bearing": bearing,
            "enabled3d": map3dEnabled,
            "buildingSourceLayer": buildingSourceLayer,
            "goongMapTilesKey": goongMapTilesKey,
            "goongStyleUrl": goongStyleUrl
        }

        web.runJavaScript("window.homeMiniMap && window.homeMiniMap.update("
                          + JSON.stringify(payload) + ");")
    }

    onCenterChanged: scheduleSync()
    onCurrentCoordinateChanged: scheduleSync()
    onDestinationCoordinateChanged: scheduleSync()
    onRoutePathChanged: {
        updateDriveSession()
        scheduleSync()
    }
    onNavigationActiveChanged: {
        updateDriveSession()
        scheduleSync()
    }
    onDriveSessionActiveChanged: {
        updateDriveSession()
        scheduleSync()
    }
    onRouteSessionSeedChanged: {
        scheduleSync()
    }
    onRouteResetSeedChanged: {
        scheduleSync()
    }
    onZoomLevelChanged: scheduleSync()
    onTiltChanged: scheduleSync()
    onBearingChanged: scheduleSync()
    onMap3dEnabledChanged: scheduleSync()
    onGoongMapTilesKeyChanged: scheduleSync()
    onGoongStyleUrlChanged: scheduleSync()
    onBuildingSourceLayerChanged: scheduleSync()
    onDriveSpeedKphChanged: {
        pushDriveSpeed()
        scheduleSync()
    }
    onStoppedSpeedThresholdKphChanged: pushDriveSpeed()
    onDriveAnimationEnabledChanged: scheduleSync()
    onFirstPersonOffsetYChanged: scheduleSync()

    Timer {
        id: syncTimer
        interval: 40
        repeat: false
        onTriggered: surface.sync()
    }

    WebEngineProfile {
        id: mapProfile

        httpUserAgent: "Mozilla/5.0 (X11; Linux aarch64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"
    }

    WebEngineView {
        id: web
        anchors.fill: parent
        profile: mapProfile
        url: surface.rendererUrl
        backgroundColor: "#050b13"

        settings.javascriptEnabled: true
        settings.localContentCanAccessFileUrls: true
        settings.localContentCanAccessRemoteUrls: true
        settings.webGLEnabled: true

        onLoadingChanged: function(loadRequest) {
            if (loadRequest.status === WebEngineView.LoadSucceededStatus) {
                console.warn("HOME_3D_MAP_WEBENGINE_LOAD_OK")
                surface.loadFailed = false
                surface.ready = true
                surface.updateDriveSession()
                surface.sync()
            } else if (loadRequest.status === WebEngineView.LoadFailedStatus) {
                surface.loadFailed = true
                console.warn("HOME_3D_MAP_WEBENGINE_LOAD_FAILED:",
                             loadRequest.errorCode,
                             loadRequest.errorString)
            }
        }

        onJavaScriptConsoleMessage: function(level, message, lineNumber, sourceID) {
            var text = String(message)
            if (text === "HOME_MAP_USER_GESTURE_START") {
                surface.userGestureStarted()
                return
            }

            if (text.indexOf("HOME_MAP_CAMERA=") === 0) {
                try {
                    surface.applyWebCamera(JSON.parse(text.substring(16)))
                } catch (e) {
                    console.warn("HOME_MAP_CAMERA_PARSE_FAILED:", e)
                }
                return
            }

            var routeProgressPrefix = "HOME_MAP_ROUTE_PROGRESS="
            if (text.indexOf(routeProgressPrefix) === 0) {
                try {
                    surface.applyRouteProgress(JSON.parse(text.substring(routeProgressPrefix.length)))
                } catch (e) {
                    console.warn("HOME_MAP_ROUTE_PROGRESS_PARSE_FAILED:", e)
                }
                return
            }

            console.warn("HOME_MAP_WEBENGINE:", level, message, sourceID, lineNumber)
        }
    }
}
