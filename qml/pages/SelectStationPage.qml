import QtQuick 2.0
import Sailfish.Silica 1.0

import StationListModel 1.0
import ProvinceListModel 1.0
import ProviderListModel 1.0

import "../items"

Page {
    id: selectStationPage

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent

        SilicaListView {
            id: listView

            model: StationListProxyModel {
                id: stationListProxyModel
                favourites: false
                provinceNameFilter: provinceListModel.selectedProvince
                provider: providerListModel.selectedProviderId
                stationModel: stationListModel
            }

            anchors.fill: parent
            header: PageHeader {
                property string provinceName: provinceListModel.selectedProvince.toLowerCase()
                title: provinceName.charAt(0).toUpperCase() + provinceName.slice(1);
            }

            delegate: StationItem {
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"))
                    stationListProxyModel.onItemClicked(index)
                }
            }

            VerticalScrollDecorator {}
        }

        BusyIndicator {
            id: loading
            anchors.centerIn: parent
            running: true
            size: BusyIndicatorSize.Large
            anchors.verticalCenter: parent.verticalCenter
            visible: true
        }

        Connections {
            target: stationListModel
            onStationListRequested: {
                loading.enabled = true
                loading.visible = true
            }
        }

        Connections {
            target: stationListModel
            onStationListLoaded: {
                loading.enabled = false
                loading.visible = false
            }
        }
    }
}
