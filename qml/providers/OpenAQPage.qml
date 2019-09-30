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

        PullDownMenu {
            MenuItem {
                text: qsTr("License")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/ProviderLicensePage.qml"))
                }
            }
        }

        Column
        {
            anchors.fill: parent

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
        }
    }
}
