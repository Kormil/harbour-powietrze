import QtQuick 2.0
import Sailfish.Silica 1.0

import StationListModel 1.0
import ProvinceListModel 1.0

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
            stationModel: stationListModel
        }

        anchors.fill: parent
        header: PageHeader {
            title: qsTr("Select station")
        }
        delegate: BackgroundItem {
            id: delegate

            Label {
                x: Theme.horizontalPageMargin
                text: model.description
                anchors.verticalCenter: parent.verticalCenter
                color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
            }
            onClicked: {
                pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"))
                stationListProxyModel.onItemClicked(index)
            }
        }
        VerticalScrollDecorator {}
    }
}
