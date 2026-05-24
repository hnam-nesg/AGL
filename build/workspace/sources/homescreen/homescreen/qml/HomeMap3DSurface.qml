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
    property bool receivingCameraFromWeb: false

    signal userGestureStarted()

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

    function sync() {
        if (!ready)
            return

        var payload = {
            "center": coordPayload(center),
            "current": coordPayload(currentCoordinate),
            "destination": coordPayload(destinationCoordinate),
            "route": routePayload(routePath),
            "navigationActive": navigationActive,
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
    onRoutePathChanged: scheduleSync()
    onNavigationActiveChanged: scheduleSync()
    onZoomLevelChanged: scheduleSync()
    onTiltChanged: scheduleSync()
    onBearingChanged: scheduleSync()
    onMap3dEnabledChanged: scheduleSync()
    onGoongMapTilesKeyChanged: scheduleSync()
    onGoongStyleUrlChanged: scheduleSync()
    onBuildingSourceLayerChanged: scheduleSync()

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
                surface.ready = true
                surface.sync()
            } else if (loadRequest.status === WebEngineView.LoadFailedStatus) {
                console.warn("HOME_3D_MAP_WEBENGINE_LOAD_FAILED:",
                             loadRequest.errorCode,
                             loadRequest.errorString)
            }
        }

        onJavaScriptConsoleMessage: function(level, message, lineNumber, sourceID) {
            if (message === "HOME_MAP_USER_GESTURE_START") {
                surface.userGestureStarted()
                return
            }

            if (String(message).indexOf("HOME_MAP_CAMERA=") === 0) {
                try {
                    surface.applyWebCamera(JSON.parse(String(message).substring(16)))
                } catch (e) {
                    console.warn("HOME_MAP_CAMERA_PARSE_FAILED:", e)
                }
                return
            }

            console.warn("HOME_MAP_WEBENGINE:", level, message, sourceID, lineNumber)
        }
    }
}
