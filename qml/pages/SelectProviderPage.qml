import QtQuick 2.0
import Sailfish.Silica 1.0

import ProviderListModel 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("Manage providers")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("ManageProvidersPage.qml"))
                }
            }
        }

        SilicaListView {
            id: listView
            model: ProviderListProxyModel {
                id: providerListProxyModel
                providerModel: providerListModel
                enabledFilter: true
            }

            anchors.fill: parent
            header: PageHeader {
                title: qsTr("Select provider")
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
                    providerListProxyModel.onItemClicked(index)
                    countryListModel.requestCountryList()
                    pageStack.push(Qt.resolvedUrl("SelectCountryPage.qml"))
                }
            }
            VerticalScrollDecorator {}
        }
    }
}
