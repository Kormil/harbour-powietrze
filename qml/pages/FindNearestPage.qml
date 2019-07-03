import QtQuick 2.2
import Sailfish.Silica 1.0

import StationListModel 1.0
import QtPositioning 5.2
import GPSModule 1.0

import "../items"

Page {
    property int count: 5

    id: page
    allowedOrientations: Orientation.All

    Component.onCompleted: {
        gps.requestPosition()
    }

    SilicaListView {

        header: PageHeader {
            id: pageHeader
            title: qsTr("Nearest station")
        }

        PushUpMenu {
            MenuItem {
                text: qsTr("Find more")
                onClicked: {
                    count = count + 5
                    gps.requestPosition()
                }
            }
        }

        id: listView
        width: parent.width
        height: parent.height
        anchors.fill: parent
        spacing: Theme.paddingLarge


        model: StationListProxyModel {
            id: stationListProxyModel
            stationModel: stationListModel
            sort: SortStation.ByDistance
            limit: count
        }

        delegate: StationNearestItem {
            name: model.description
            distance: model.distance + " km"
            onClicked: {
                pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"))
                stationListProxyModel.onItemClicked(index)
            }
        }

        VerticalScrollDecorator {}
    }

    ProgressBar {
        id: loading
        indeterminate: true
        anchors.bottom: parent.bottom
        width: parent.width
        visible: false
        label: qsTr("Wait for GPS")
    }

    Connections {
        target: gps
        onPositionRequested: {
            loading.visible = true
        }
    }

    Connections {
        target: stationListModel
        onNearestStationFounded: {
            loading.visible = false
        }
    }

}
