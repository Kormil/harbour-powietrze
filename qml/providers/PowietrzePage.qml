import QtQuick 2.0
import Sailfish.Silica 1.0
import Settings 1.0

import ProviderListModel 1.0
import "../items"

Page {
    property var provider: providerListModel.selectedProvider
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    SilicaFlickable{
        anchors.fill: parent
        anchors.leftMargin: Theme.paddingMedium
        anchors.rightMargin: Theme.paddingMedium
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: qsTr("License")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/ProviderLicensePage.qml"))
                }
            }
        }

        VerticalScrollDecorator {}

        Column {
            id: column
            width: parent.width

            PageHeader {
                id: header
                title: provider ? provider.name : ""
            }

            ProviderItem {
                provider: page.provider
            }

            TextSwitch {
                text: qsTr("Enabled")
                checked: provider ? provider.enabled : false

                onCheckedChanged: {
                    if (provider && provider.enabled !== checked) {
                        provider.enabled = checked
                    }
                }
            }

            ComboBox {
                width: page.width
                label: qsTr("Pollution names")
                currentIndex: provider.nameVariant

                menu: ContextMenu {
                    MenuItem { text: qsTr("Normal") }
                    MenuItem { text: qsTr("Short") }
                }

                onCurrentIndexChanged: {
                    if (provider && provider.nameVariant !== currentIndex) {
                        provider.nameVariant = currentIndex
                    }
                }
            }

            ComboBox {
                width: page.width
                label: qsTr("Air quality index")
                currentIndex: provider.airQualityIndex

                menu: ContextMenu {
                    MenuItem { text: "Polski indeks jakości powietrza" }
                    MenuItem { text: "European air quality index" }
                }

                onCurrentIndexChanged: {
                    if (provider && provider.airQualityIndex !== currentIndex) {
                        provider.airQualityIndex = currentIndex
                    }
                }
            }
        }
    }
}
