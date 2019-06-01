import QtQuick 2.0
import Sailfish.Silica 1.0
import Settings 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
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

            Text {
                id: licenseText
                width: parent.width
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                text: Settings.license()
                wrapMode: Text.Wrap
            }
        }

        VerticalScrollDecorator {}
    }
}
