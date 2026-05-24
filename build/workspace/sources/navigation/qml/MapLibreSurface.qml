import QtQuick
import QtWebEngine
import QtPositioning

Item {
    id: surface

    property var centerCoordinate
    property var currentCoordinate
    property var destinationCoordinate
    property var routePath: []
    property real zoomLevel: 14
    property real pitch: 52
    property real bearing: 0
    property bool map3dEnabled: true
    property bool followExternalCenter: true

    property string rendererUrl: "qrc:/goong-map.html"
    property string goongMapTilesKey: "Xgw2Eiqb38KmKuSsxQFH5c4NtEuORfbAdWfbxXIB"
    property string goongStyleUrl: "https://tiles.goong.io/assets/goong_map_web.json"
    property string vectorTileUrl: "http://127.0.0.1:8080/data/v3.json"
    property string buildingSourceLayer: "VN_Building"

    property bool ready: false
    property bool receivingCameraFromWeb: false

    property var map: mapObject

    signal userGestureStarted()

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

    function sync() {
        if (!ready)
            return

        var payload = {
            "center": coordPayload(map.center || centerCoordinate),
            "current": coordPayload(currentCoordinate),
            "destination": coordPayload(destinationCoordinate),
            "route": routePayload(routePath),
            "zoom": map.zoomLevel,
            "pitch": map3dEnabled ? map.tilt : 0,
            "bearing": map.bearing,
            "enabled3d": map3dEnabled,
            "vectorTileUrl": vectorTileUrl,
            "buildingSourceLayer": buildingSourceLayer,
            "goongMapTilesKey": goongMapTilesKey,
            "goongStyleUrl": goongStyleUrl
        }

        web.runJavaScript("window.navigationMap && window.navigationMap.update("
                          + JSON.stringify(payload) + ");")
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

    onCurrentCoordinateChanged: scheduleSync()
    onDestinationCoordinateChanged: scheduleSync()
    onRoutePathChanged: scheduleSync()

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
    onVectorTileUrlChanged: scheduleSync()
    onBuildingSourceLayerChanged: scheduleSync()
    onGoongMapTilesKeyChanged: scheduleSync()
    onGoongStyleUrlChanged: scheduleSync()

    Component.onCompleted: {
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

            console.warn("WEBENGINE_CONSOLE:", level, message, sourceID, lineNumber)
        }
    }
}
