import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning
import Qt5Compat.GraphicalEffects
import QtQuick.VirtualKeyboard
import QtQuick.VirtualKeyboard.Settings
import QtQuick.Controls.Basic as Control
import QtQuick.Effects
import "helper.js" as Helper

ApplicationWindow {
    id: view
    y:5; width: 860; height: 660

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

    function contentRoute(){
        routeInfoModel.clear()
        if (routeModel.count > 0) {
            // for (var i = 0; i < routeModel.get(0).segments.length; i++) {
            //     routeInfoModel.append({
            //                               "instruction": routeModel.get(0).segments[i].maneuver.instructionText,
            //                               "distance": Helper.formatDistance(routeModel.get(0).segments[i].maneuver.distanceToNextInstruction)
            //                           });
        }
        ins.contentText = routeModel.get(0).segments[0].maneuver.instructionText //+ Helper.formatDistance(routeModel.get(0).segments[0].maneuver.distanceToNextInstruction)
        distance_target.contentText = Helper.formatDistance(routeModel.get(0).segments[0].maneuver.distanceToNextInstruction)
        time.contentText = (routeModel.count === 0 ? "" : Helper.formatTime(routeModel.get(0).travelTime))
        distance.contentText = (routeModel.count === 0 ? "" : Helper.formatDistance(routeModel.get(0).distance))

        if (/turn left/i.test(routeModel.get(0).segments[1].maneuver.instructionText)) image_turn.source = ASSET_BASE + "asset/icons8-undo-94.png"
        else if (/turn right/i.test(routeModel.get(0).segments[1].maneuver.instructionText)) image_turn.source = ASSET_BASE + "asset/icons8-redo-94.png"
        else image_turn.source = ASSET_BASE + "asset/icons8-arrow-up-94.png"
    }

    function fitRoute() {
        var minLat = Math.min(root.startCoord.latitude, root.destCoord.latitude)
        var maxLat = Math.max(root.startCoord.latitude, root.destCoord.latitude)
        var minLon = Math.min(root.startCoord.longitude, root.destCoord.longitude)
        var maxLon = Math.max(root.startCoord.longitude, root.destCoord.longitude)

        var centerLat = (minLat + maxLat) / 2
        var centerLon = (minLon + maxLon) / 2

        mapView.map.center = QtPositioning.coordinate(centerLat, centerLon)

        var latDiff = maxLat - minLat
        var lonDiff = maxLon - minLon

        latDiff *= 1.4
        lonDiff *= 1.4

        var maxDiff = Math.max(latDiff, lonDiff)
        var zoomLevel = 10
        console.log(maxDiff)

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

        mapView.map.zoomLevel = zoomLevel
    }

    FontLoader{
        id: font_name
        source: ASSET_BASE + "asset/Roboto-BoldCondensed.ttf"
    }

    Item{
        id: root
        x: 20; y: 20; width: 810; height: 620//anchors.fill: parent

        property var startCoord: QtPositioning.coordinate(10.870875835378296, 106.79419965740207)
        property var destCoord:  QtPositioning.coordinate(10.77300, 106.70400)

        PositionSource {
            id: ps
            active: true
            updateInterval: 1000
            onPositionChanged: if (ps.valid) mapView.map.center = position.coordinate
        }
        property bool hasLivePos: false//ps.valid
        property var  currentCoord: root.hasLivePos ? ps.position.coordinate : root.startCoord
        property real currentHeading: 0

        Plugin {
            id: osm
            name: "osm"
            PluginParameter { name: "osm.useragent"; value: "IVI_AGL/1.0/hoainam16119@gmail.com" }
            PluginParameter { name: "osm.mapping.host"; value: "https://tile.openstreetmap.org" }
            PluginParameter { name: "osm.mapping.providersrepository.enabled"; value: true }
            PluginParameter { name: "osm.mapping.highdpi_tiles"; value: true }
            PluginParameter { name: "osm.mapping.providersrepository.address"; value: "http://127.0.0.1:8000/" }
            PluginParameter { name: "osm.routing.osrm.server";  value: "https://router.project-osrm.org" }
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


        // function shortestBearing(from, to) {
        //         let d = (to - from) % 360;
        //         if (d < -180) d += 360;
        //         if (d >  180) d -= 360;
        //         return from + d;
        //     }

        // function moveAhead(coord, bearingDeg, distM) {
        //        const R = 6371000.0;            // bán kính Trái đất (m)
        //        const br = bearingDeg * Math.PI/180.0;
        //        const lat1 = coord.latitude  * Math.PI/180.0;
        //        const lon1 = coord.longitude * Math.PI/180.0;
        //        const dR = distM / R;

        //        const lat2 = Math.asin(Math.sin(lat1)*Math.cos(dR) + Math.cos(lat1)*Math.sin(dR)*Math.cos(br));
        //        const lon2 = lon1 + Math.atan2(Math.sin(br)*Math.sin(dR)*Math.cos(lat1),
        //                                       Math.cos(dR)-Math.sin(lat1)*Math.sin(lat2));
        //        return QtPositioning.coordinate(lat2*180/Math.PI, lon2*180/Math.PI);
        //    }

        // function setFirstPerson() {
        //        // nghiêng mạnh + zoom gần
        //        mapView.map.tilt = 60;
        //        mapView.map.zoomLevel = 20;

        //        // xoay bản đồ theo hướng xe (animate theo đường ngắn nhất)
        //        mapView.map.bearing = shortestBearing(mapView.map.bearing, root.currentHeading);

        //        // đẩy center lên trước đầu xe ~120m để “thấy phía trước”
        //        const ahead = moveAhead(root.currentCoord, root.currentHeading, 120);
        //        mapView.map.center = ahead;
        //    }

        //    // Ví dụ cập nhật vị trí/heading (thay bằng PositionSource của bạn)
        // Timer {
        //     interval: 1000; running: true; repeat: true
        //     onTriggered: {
        // demo quay nhẹ
        // root.currentHeading = (root.currentHeading + 10) % 360
        // // nếu đang ở first-person, giữ cho map theo xe
        // if (mapView.map.tilt > 50) {
        //     mapView.map.bearing = root.shortestBearing(mapView.map.bearing, root.currentHeading)
        //     //mapView.map.center = root.moveAhead(mapView.currentCoord, root.destCoord, 120)
        // }
        //mapView.map.bearing += root.angleRouteVsScreenY(root.currentCoord, 0)
        //mapView.map.center = root.currentCoord
        //     }
        // }

        MapView {
            id: mapView
            anchors.fill: parent
            map.plugin: osm
            map.center: root.currentCoord
            map.zoomLevel: nameSearch ? 15 : 20
            map.tilt: nameSearch.visible ? 0 : 60
            //map.bearing: 0

            map.activeMapType: map.supportedMapTypes[1]


            Behavior on height{NumberAnimation{duration: 200; easing.type: Easing.Linear}}

            Behavior on map.center{CoordinateAnimation{duration: 1000; easing.type: Easing.InOutQuad}}

            Behavior on map.zoomLevel{NumberAnimation{duration: 200; easing.type: Easing.Linear}}

            PinchHandler{
                id: pinch
                target: null
                onScaleChanged: {
                    mapView.map.zoomLevel = Math.min(20, Math.max(1, mapView.zoomLevel + Math.log(scale)))
                }
            }

            // Marker hiện tại / start
            MapQuickItem {
                parent: mapView.map
                coordinate: root.hasLivePos ? root.currentCoord : root.startCoord
                anchorPoint.x: 12; anchorPoint.y: 24
                sourceItem: Rectangle { width: 24; height: 24; radius: 12; color: "#14b8a6"; border.color: "white"; border.width: 2 }
            }
            // Marker đích
            MapQuickItem {
                parent: mapView.map
                coordinate: root.destCoord
                anchorPoint.x: 12; anchorPoint.y: 24
                sourceItem: Rectangle { width: 24; height: 24; radius: 12; color: "#ef4444"; border.color: "white"; border.width: 2 }
            }
            // VẼ TUYẾN – chỉ 1 MapRoute, gán khi Ready

            MapRoute {
                id: activeRoute
                parent: mapView.map
                route: routeModel.count > 0 ? routeModel.get(0) : null
                visible: routeModel.count > 0
                line.width: 6
                line.color: "#3b82f6"
            }

            MapPolyline {
                id: fallbackLine
                parent: mapView.map
                line.width: 8
                line.color: "#0038ff"
                path: []
            }

            // MouseArea {
            //     anchors.fill: parent
            //     acceptedButtons: Qt.LeftButton
            //     onClicked: (m) => {
            //                    root.startCoord = mapView.map.toCoordinate(Qt.point(m.x, m.y))
            //                    mapView.map.center = root.startCoord
            //                    routeCtl.requestRoute()
            //                    Qt.inputMethod.hide()
            //                    nameSearch.focus = false
            //                }
            //     onPressAndHold: (m) => {
            //                         root.startCoord = mapView.map.toCoordinate(Qt.point(m.x, m.y))
            //                         mapView.map.center = root.startCoord
            //                     }
            // }
        }

        RouteQuery { id: routeQuery }
        RouteModel {
            id: routeModel
            plugin: mapView.map.plugin
            query: routeQuery
            onStatusChanged: {
                if (status == RouteModel.Ready) {
                    switch (count) {
                    case 0:
                        view.routeError()
                        break
                    case 1:
                        contentRoute()
                        break
                    }
                } else if (status == RouteModel.Error) {
                    view.routeError()
                }
            }
        }

        // Geocoding tên → root.destCoord
        GeocodeModel {
            id: geocoder
            plugin: mapView.map.plugin
            onErrorChanged: if (error !== GeocodeModel.NoError) console.warn("Geocode error:", errorString)
            onCountChanged: {
                if (count > 0) {
                    const c = get(0).coordinate
                    root.destCoord = c
                    //mapView.map.center = c
                    routeCtl.requestRoute()
                    view.fitRoute()
                } else {
                    console.warn("Geocode: no results")
                }
            }
        }

        QtObject {
            id: httpRoute
            function drawCarRoute(start, dest) {
                const url = `https://router.project-osrm.org/route/v1/driving/${start.longitude},${start.latitude};${dest.longitude},${dest.latitude}?overview=full&geometries=geojson`
                const xhr = new XMLHttpRequest()
                xhr.onreadystatechange = function() {
                    if (xhr.readyState === XMLHttpRequest.DONE) {
                        try {
                            const j = JSON.parse(xhr.responseText)
                            if (j.code === "Ok" && j.routes && j.routes.length > 0) {
                                const coords = j.routes[0].geometry.coordinates // [lon,lat][]
                                const pts = []
                                for (let i=0; i<coords.length; ++i)
                                    pts.push(QtPositioning.coordinate(coords[i][1], coords[i][0]))
                                fallbackLine.path = pts
                                activeRoute.visible = false
                            } else {
                                console.warn("OSRM fallback: no route", xhr.responseText)
                                fallbackLine.path = []
                            }
                        } catch (e) {
                            console.warn("OSRM fallback parse error:", e, xhr.responseText)
                            fallbackLine.path = []
                        }
                    }
                }
                xhr.open("GET", url)
                xhr.send()
            }
        }

        //Routing

        QtObject {
            id: routeCtl
            property bool busy: false

            function requestRoute() {
                if (busy) return;
                busy = true;
                const start = root.startCoord
                console.log("Routing start=", start.latitude, start.longitude,
                            "dest=", root.destCoord.latitude, root.destCoord.longitude)

                routeQuery.clearWaypoints()
                routeQuery.addWaypoint(start)
                routeQuery.addWaypoint(root.destCoord)
                routeQuery.travelModes = RouteQuery.CarTravel
                routeQuery.routeOptimizations = RouteQuery.ShortestRoute

                routeModel.update()
                mapView.map.center = root.startCoord;
            }

            //Component.onCompleted: requestRoute()
        }
        Connections {
            target: routeModel

            function onStatusChanged() {
                httpRoute.drawCarRoute(root.hasLivePos ? root.currentCoord : root.startCoord, root.destCoord)
                routeCtl.busy = false
            }
        }

        ListModel { id: suggestionModel }

        Column {
            x: 20; y: 20; z: 1; width: 400; height: 40

            Control.TextField {
                id: nameSearch
                x: 20; y: 20; width: 400; height: 40
                onAccepted: { geocoder.query = text; geocoder.update(); Qt.inputMethod.hide(); focus = false; suggestionModel.clear()}
                onTextEdited: {SearchController.scheduleSearch(text, root.currentCoord.latitude, root.currentCoord.longitude)}
                EnterKeyAction.actionId: EnterKeyAction.Done
                font.family: "Arial"; font.styleName: "Italic"; color: "black"
                background: Rectangle{
                    anchors.fill: parent
                    color: "#8aa7a7a7"
                    border.color: Qt.darker(color, 1.1)
                    layer.enabled: true
                    layer.effect: DropShadow {
                        horizontalOffset: 0; verticalOffset: 0
                        samples: 20; color: "#80FFFFFF"
                    }
                }
            }
            Rectangle{
                x: 20; width: 400
                height: Math.min(suggestionView.contentHeight, 260)
                color: "white"
                ListView {
                    id: suggestionView
                    model: suggestionModel
                    visible: count > 0
                    width: 400
                    height: Math.min(contentHeight, 260)
                    clip: true

                    delegate: ItemDelegate {
                        width: 400
                        text: primary
                        contentItem: Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text {
                                text: primary
                                font.bold: true
                                elide: Text.ElideRight
                            }
                            Text {
                                text: secondary
                                font.pixelSize: 11
                                color: "#666666"
                                elide: Text.ElideRight
                            }
                        }
                        onClicked: { geocoder.query = primary; geocoder.update(); Qt.inputMethod.hide(); focus = false; nameSearch.text = primary; suggestionModel.clear()}
                    }
                }
            }
        }

        InputPanel {
            id: inputPanel
            z: 1
            y: yPositionWhenHidden
            x: 0
            width: mapView.width

            property real yPositionWhenHidden: mapView.height + 300

            states: State {
                name: "visible"
                when: inputPanel.active
                PropertyChanges {
                    target: inputPanel
                    y: inputPanel.yPositionWhenHidden - inputPanel.height - 300
                }
            }
            transitions: Transition {
                id: inputPanelTransition
                from: ""
                to: "visible"
                reversible: true
                enabled: !VirtualKeyboardSettings.fullScreenMode
                ParallelAnimation {
                    NumberAnimation {
                        properties: "y"
                        duration: 250
                        easing.type: Easing.InOutQuad
                    }
                }
            }
            Binding {
                target: InputContext
                property: "animating"
                value: inputPanelTransition.running
                restoreMode: Binding.RestoreBinding
            }
        }

        ButtonControl {
            id: myLocation
            x: 750; y: nameSearch.visible ? 480 : 280; width: 40; height: 40; source: ASSET_BASE + "asset/icons8-my-location-48.png"
            background_: true
            onPressed: mapView.map.center = root.currentCoord
        }

        ButtonControl {
            id: zoomPlus
            x: 750; y: nameSearch.visible ? 530 : 330; width: 40; height: 40; source: ASSET_BASE + "asset/icons8-plus-48.png"
            background_: true
            onClicked: {
                mapView.map.zoomLevel += 1
                console.log(mapView.map.zoomLevel)
            }
        }

        ButtonControl {
            id: zoomSubtract
            x: 750; y: nameSearch.visible ? 580 : 380; width: 40; height: 40; source: ASSET_BASE + "asset/icons8-subtract-48.png"
            background_: true
            onClicked: mapView.map.zoomLevel -= 1
        }

        ButtonControl {
            id: search
            x: 440; y: 20; z: 1; width: 40; height: 40; source: ASSET_BASE + "asset/icons8-search-48.png"
            background_: true
            onPressed: {
                //Behavior on mapView.map.zoomLevel{NumberAnimation{duration: 1000; easing.type: Easing.InOutBack}}
                routeCtl.requestRoute()
                //view.fitRoute()
                display_route.y = 440
                mapView.height = 600
                search.visible = false; nameSearch.visible = false
                mapView.map.bearing = 90
                mapView.map.zoomLevel = 20
                mapView.map.center = root.startCoord
            }
        }

        Rectangle {
            id: display_route
            x: 10; y: 680; width: mapView.width - 20; height: 190; color: "black"; radius: 20
            Behavior on y{NumberAnimation{duration: 200; easing.type: Easing.InOutBack}}

            ButtonControl {
                id: time_distance
                x: 20; y: 15; width: 180; height: 80; radius_bg: 15; background_: true; color_bg: "#202a36"; enabled: false

                TextEffect {
                    id: time
                    x: 0; width: 180; height: 40; contentText: qsTr(""); fontSize: 30
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                TextEffect {
                    id: distance
                    x: 0; y: 40; width: 90; height: 40; contentText: qsTr(""); fontSize: 22
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                }
                Timer {
                    id: timer
                    interval: 1000
                    running: true
                    repeat: true
                    onTriggered: {
                        time_estimate.contentText = Qt.formatTime(new Date(Date.now() + Number(routeModel.get(0).travelTime)*1000), "hh:mm")
                    }
                }
                TextEffect {
                    id: time_estimate
                    x: 110; y: 40; width: 70; height: 40; contentText: Qt.formatTime(new Date(Date.now() + Number(routeModel.get(0).travelTime)*1000), "hh:mm"); fontSize: 22
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Rectangle {
                id: after_guide
                x: 220; y: 105; width: 200; height: 70; color: "#202a36"; radius: 15
                TextEffect {
                    x: 0; y: 15; width: 100; height: 50; contentText: qsTr("After"); fontSize: 20
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                }

                Image {
                    id: image_turn
                    x: 130; y: 15; width: 50; height: 50
                    source: ASSET_BASE + "asset/icons8-redo-94.png"; fillMode: Image.PreserveAspectFit
                }
            }

            Rectangle {
                id: current_guide
                x: 220; y: 15; width: 570; height: 100; color: "#222e42"; radius: 15

                TextEffect {
                    id: ins
                    x: 90; y: 20; width: 450; height: 60; contentText: qsTr(""); fontSize: 16; wrapMode_: Text.Wrap
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                }

                TextEffect {
                    id: distance_target
                    x: 20; y: 50; width: 60; height: 40; contentText: qsTr(""); fontSize: 20
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Image {
                    x: 30; y: 10; width: 40; height: 40
                    source: ASSET_BASE + "asset/icons8-arrow-up-94.png"
                    fillMode: Image.PreserveAspectFit
                }

                Rectangle {
                    x: 0; y: 80; width: 200; height: 20; color: "#222e42"
                }
            }


            ButtonControl {
                id: quit
                x: 60; y: 115; width: 100; height: 60; contentText: "Quit"; fontSize: 30; background_: true; color_bg: "#202a36"; radius_bg: 15
                onPressed: { mapView.height = 800; display_route.y = 800; nameSearch.visible = true; search.visible = true}
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
        }
    }

}
