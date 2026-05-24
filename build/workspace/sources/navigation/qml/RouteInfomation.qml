
import QtQuick
import QtQuick.Controls
import "helper.js" as Helper

ListView {
    property variant routeModel
    property string totalTravelTime
    property string totalDistance
    signal closeForm()
    interactive: true
    model: ListModel { id: routeInfoModel }
    header: RouteListHeader {
        travelTime.text: totalTravelTime
        distance.text: totalDistance
    }
    delegate:  RouteListDelegate{
        routeIndex.text: index + 1
        routeInstruction.text: instruction
        routeDistance.text: distance
    }
    footer: Button {
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("Close")
        onClicked: {
            closeForm()
        }
    }

    function refreshRouteInfo() {
        routeInfoModel.clear()
        if (!routeModel || routeModel.count === 0) {
            totalTravelTime = ""
            totalDistance = ""
            return
        }

        var route = routeModel.get(0)
        var segments = route.segments || []
        for (var i = 0; i < segments.length; i++) {
            var maneuver = segments[i].maneuver || {}
            routeInfoModel.append({
                "instruction": maneuver.instructionText || "",
                "distance": Helper.formatDistance(maneuver.distanceToNextInstruction)
            });
        }
        totalTravelTime = Helper.formatTime(route.travelTime)
        totalDistance = Helper.formatDistance(route.distance)
    }

    Component.onCompleted: refreshRouteInfo()

    onRouteModelChanged: refreshRouteInfo()

    Connections {
        target: routeModel || null
        ignoreUnknownSignals: true
        function onStatusChanged() {
            refreshRouteInfo()
        }
    }
}
