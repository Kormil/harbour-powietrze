import QtQuick 2.2
import Sailfish.Silica 1.0

import StationListModel 1.0
import QtPositioning 5.2

Page {
    property int count: 5
    property int verticalAccuracy: 100
    property int horizontalAccuracy: 100

    id: page
    allowedOrientations: Orientation.All

    PositionSource {
        id: positionSource
        updateInterval: 1000
        active: true
    }

    function findNearestStation() {
        loading.visible = true

        console.log("GPS: verticalAccuracy: " + positionSource.position.verticalAccuracy)
        console.log("GPS: horizontalAccuracy: " + positionSource.position.horizontalAccuracy)

        if (!positionSource.position.verticalAccuracyValid || positionSource.position.verticalAccuracy > verticalAccuracy)
            return;

        if (!positionSource.position.horizontalAccuracyValid || positionSource.position.horizontalAccuracy > horizontalAccuracy)
            return;

        if (stationListModel.findDistances(positionSource.position.coordinate)) {
            stationListProxyModel.invalidate()
            loading.visible = false
            positionSource.stop()
        }
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
                    findNearestStation()
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
            favourites: false
            stationModel: stationListModel
            sort: SortStation.ByDistance
            limit: count
        }

        delegate: BackgroundItem {
            id: delegate
            contentHeight: Theme.itemSizeSmall

            Label {
                id: nameLabel
                text: model.description
                anchors.left: parent.left
                anchors.leftMargin: Theme.horizontalPageMargin
                anchors.right: distanceLabel.left
                color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                truncationMode: TruncationMode.Fade
            }

            Label {
                id: distanceLabel
                anchors.right: parent.right
                anchors.rightMargin: Theme.horizontalPageMargin
                anchors.leftMargin: Theme.horizontalPageMargin
                text: model.distance + " km"
                font.pixelSize: Theme.fontSizeExtraSmall
                color: delegate.highlighted ? Theme.highlightColor : Theme.highlightColor
            }

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
        label: qsTr("Wait for GPS")
    }

    Connections {
        target: positionSource
        onPositionChanged: {
            findNearestStation()
        }

    }
}
