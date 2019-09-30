import QtQuick 2.0
import Sailfish.Silica 1.0
import Settings 1.0

import ProviderListModel 1.0

Page {
    property var provider: providerListModel.selectedProvider

    id: page
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        anchors.leftMargin: Theme.paddingMedium
        anchors.rightMargin: Theme.paddingMedium
        contentHeight: column.height

        Column {
            id: column
            width: parent.width

            PageHeader {
                title: qsTr("License")
            }

            LinkedLabel {
                id: licenseText
                width: parent.width
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                plainText: provider ? Settings.providerLicense(provider.name) : ""
                wrapMode: Text.Wrap
            }
        }

        VerticalScrollDecorator {}
    }
}
