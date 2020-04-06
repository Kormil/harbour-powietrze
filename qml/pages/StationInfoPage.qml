import QtQuick 2.2
import Sailfish.Silica 1.0

import StationListModel 1.0
import SensorListModel 1.0
import ProviderListModel 1.0
import Settings 1.0

import "../items"

Page {
    id: page
    allowedOrientations: Orientation.All

    onStatusChanged: {
        if (status === PageStatus.Deactivating) {
            changeCoverPage(Qt.resolvedUrl("../cover/FavouriteCoverPage.qml"));
        }
    }

    SilicaFlickable {
        property int heightWithProviderLabel: station.height + prviderLabel.height

        anchors.fill: parent
        contentHeight: page.height > heightWithProviderLabel ? page.height : heightWithProviderLabel + Theme.paddingMedium

        PullDownMenu {
            MenuItem {
                id: addFevourite
                text: qsTr("Add to favourite")
                visible: false
                onClicked: {
                    stationListModel.selectedStation.favourite = true
                    Settings.addFavouriteStation(stationListModel.selectedStation)
                }
            }

            MenuItem {
                id: removeFevourite
                text: qsTr("Remove from favourite")
                visible: false
                onClicked: {
                    stationListModel.selectedStation.favourite = false
                    Settings.removeFavouriteStation(stationListModel.selectedStation.id)
                }
            }
        }

        VerticalScrollDecorator {}

        Column {
            id: station
            width: parent.width

            StationInfoItem {
                id: stationInfo
                station: stationListModel.selectedStation
                enabled: false
                anchors.margins: Theme.horizontalPageMargin
            }

            SilicaListView {
                id: listView
                model: sensorListModel

                width: page.width
                height: contentHeight

                delegate: SensorItem {
                    sensor: model;
                }
            }
        }

        Label {
            id: prviderLabel
            width:  parent.width
            anchors.bottom: parent.bottom
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.secondaryColor
            anchors.bottomMargin: Theme.paddingSmall
            horizontalAlignment: Text.AlignHCenter
        }

        Connections {
            target: stationListModel
            onStationDataRequested: {
                loading.enabled = true
                loading.visible = true
                station.enabled = false
                station.visible = false
            }
        }

        Connections {
            target: stationListModel
            onStationDataLoaded: {
                loading.enabled = false
                loading.visible = false
                station.enabled = true
                station.visible = true

                addFevourite.visible = !stationListModel.selectedStation.favourite
                removeFevourite.visible = stationListModel.selectedStation.favourite

                changeCoverPage(Qt.resolvedUrl("../cover/StationCoverPage.qml"));

                prviderLabel.text = qsTr("Data by ") + providerListModel.site(stationListModel.selectedStation.provider)
            }
        }

        Connections {
            target: stationListModel
            onDataChanged: {
                addFevourite.visible = !stationListModel.selectedStation.favourite
                removeFevourite.visible = stationListModel.selectedStation.favourite
            }
        }
    }

    BusyIndicator {
        id: loading
        anchors.centerIn: parent
        running: true
        size: BusyIndicatorSize.Large
        anchors.verticalCenter: parent.verticalCenter
        visible: false
    }
}
