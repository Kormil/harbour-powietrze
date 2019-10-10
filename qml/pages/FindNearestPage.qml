import QtQuick 2.2
import Sailfish.Silica 1.0

import StationListModel 1.0
import QtPositioning 5.2
import GPSModule 1.0

import "../items"

Page {
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
                id: findMoreItem
                text: qsTr("Find more")
                visible: false
                onClicked: {
                    stationListModel.stationDistanceLimit = stationListModel.stationDistanceLimit * 2
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
            distanceLimit: stationListModel.stationDistanceLimit
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
            findMoreItem.visible = false
        }
    }

    Connections {
        target: gps
        onPositionFounded: {
            loading.visible = false
            findMoreItem.visible = true
        }
    }

}
