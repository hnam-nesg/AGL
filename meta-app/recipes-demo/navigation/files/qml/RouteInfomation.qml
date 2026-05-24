
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
    header: RouteListHeader {}
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

    Component.onCompleted: {
        routeInfoModel.clear()
        if (routeModel.count > 0) {
            for (var i = 0; i < routeModel.get(0).segments.length; i++) {
                routeInfoModel.append({
                    "instruction": routeModel.get(0).segments[i].maneuver.instructionText,
                     "distance": Helper.formatDistance(routeModel.get(0).segments[i].maneuver.distanceToNextInstruction)
                });
            }
        }
        totalTravelTime = routeModel.count == 0 ? "" : Helper.formatTime(routeModel.get(0).travelTime)
        totalDistance = routeModel.count == 0 ? "" : Helper.formatDistance(routeModel.get(0).distance)
    }
}
