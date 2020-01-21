import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    property var station : undefined

    anchors.centerIn: parent
    width: parent.width
    height: parent.height
    visible: station !== undefined

    Label {
        id: pageHeader
        text: station.cityName
        font.pixelSize: Theme.fontSizeMedium
        color: Theme.primaryColor
        x: Theme.horizontalPageMargin
        truncationMode: TruncationMode.Fade
        width: parent.width
    }

    Label {
        id: streetName
        anchors.top: pageHeader.bottom
        text: station.streetName
        font.pixelSize: Theme.fontSizeExtraSmall
        color: Theme.primaryColor
        x: Theme.horizontalPageMargin
        truncationMode: TruncationMode.Fade
        width: parent.width
    }

    Item {
        id: index
        width: parent.width
        height: image.height + label.height
        anchors.centerIn: parent

        Image {
            id: image
            width: parent.width / 2
            height: width
            source: "qrc:///Graphics/weatherIcons/256/" + station.stationIndex.id + ".png"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            id: label
            text: station.stationIndex.name
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: Theme.paddingSmall
            anchors.top: image.bottom
            width: parent.width
            truncationMode: TruncationMode.Fade
        }
    }
}
