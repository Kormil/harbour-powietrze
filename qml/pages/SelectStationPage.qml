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
            title: qsTr("Select station")
        }

        delegate: StationItem {
            onClicked: {
                pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"))
                stationListProxyModel.onItemClicked(index)
            }
        }
        VerticalScrollDecorator {}
    }
}
