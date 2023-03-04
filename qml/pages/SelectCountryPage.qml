import QtQuick 2.0
import Sailfish.Silica 1.0

import CountryListModel 1.0
import ProvinceListModel 1.0
import ProviderListModel 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("Find nearest stations")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("FindNearestPage.qml"), {providerSelected: true})
                }
            }
        }

        SilicaListView {
            id: listView
            model: CountryListProxyModel {
                id: countryListProxyModel
                countryModel: countryListModel
                provider: providerListModel.selectedProviderId
            }

            anchors.fill: parent
            header: PageHeader {
                title: qsTr("Select country")
            }
            delegate: BackgroundItem {
                id: delegate

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - Theme.horizontalPageMargin * 2
                    text: model.name
                    anchors.verticalCenter: parent.verticalCenter
                    color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                    truncationMode: TruncationMode.Fade
                }
                onClicked: {
                    countryListProxyModel.onItemClicked(index)
                    pageStack.push(Qt.resolvedUrl("SelectProvincePage.qml"))
                    provinceListModel.requestProvinceList()
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
            target: countryListModel
            onCountryListRequested: {
                loading.enabled = true
                loading.visible = true
            }
        }

        Connections {
            target: countryListModel
            onCountryListLoaded: {
                loading.enabled = false
                loading.visible = false
            }
        }
    }
}
