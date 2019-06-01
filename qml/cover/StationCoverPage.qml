import QtQuick 2.0
import Sailfish.Silica 1.0

import StationListModel 1.0

CoverBackground {
    Label {
        id: pageHeader
        text: stationListModel.selectedStation.cityName
        font.pixelSize: Theme.fontSizeMedium
        color: Theme.primaryColor
        x: Theme.horizontalPageMargin
        truncationMode: TruncationMode.Fade
        width: parent.width
    }

    Label {
        id: streetName
        anchors.top: pageHeader.bottom
        text: stationListModel.selectedStation.streetName
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
            source: "qrc:///Graphics/weatherIcons/256/" + stationListModel.selectedStation.stationIndex.id + ".png"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Label {
            id: label
            text : stationListModel.selectedStation.stationIndex.name
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: Theme.paddingSmall
            anchors.top: image.bottom
        }
    }
}

