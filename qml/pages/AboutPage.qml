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

        VerticalScrollDecorator {}

        Column {
            id: column
            spacing: Theme.paddingLarge
            width: parent.width

            PageHeader {
                title: qsTr("About")
            }

            Image {
                id: appIconImage
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                height: 172
                source: "qrc:///icons/172x172/harbour-powietrze.png"
            }

            Label {
                id: appNameLabel
                width: parent.width
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: Theme.fontSizeExtraLarge
                color: Theme.primaryColor
                text: "Powietrze"
            }

            Label {
                id: appVersionLabel
                width: parent.width
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                text: qsTr("version") + " 1.2.0"
            }

            Label {
                id: appDescriptionLabel
                width: parent.width
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Application for checking the current state of air quality.")
            }

            SectionHeader {
                text: "Copyright"
            }

            Label {
                width: parent.width
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: Theme.fontSizeSmall
                horizontalAlignment: Text.AlignHCenter
                text: "Bartłomiej Seliga"
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("License")
                onClicked: pageStack.push(Qt.resolvedUrl("LicensePage.qml"))
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Source code")
                onClicked: Qt.openUrlExternally("https://github.com/Kormil/harbour-powietrze")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Buy me a beer")
                onClicked: Qt.openUrlExternally("https://paypal.me/kormil")
            }
        }
    }
}
