import QtQuick 2.0
import Sailfish.Silica 1.0

import ProvinceListModel 1.0
import "."

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All   

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("Show all")
                onClicked: {
                    provinceListModel.onItemClicked(-1)
                    pageStack.push(Qt.resolvedUrl("SelectStationPage.qml"))
                }
            }
        }

        SilicaListView {
            id: listView
            model: provinceListModel

            anchors.fill: parent
            header: PageHeader {
                title: qsTr("Select province")
            }
            delegate: BackgroundItem {
                id: delegate

                Label {
                    x: Theme.horizontalPageMargin
                    text: model.name
                    anchors.verticalCenter: parent.verticalCenter
                    color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                }
                onClicked: {
                    provinceListModel.onItemClicked(index)
                    onClicked: pageStack.push(Qt.resolvedUrl("SelectStationPage.qml"))
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
        }

        Connections {
            target: provinceListModel
            onProvinceLoaded: {
                loading.enabled = false
                loading.visible = false
            }
        }
    }
}
