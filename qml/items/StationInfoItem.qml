import QtQuick 2.0
import Sailfish.Silica 1.0

import StationListModel 1.0

BackgroundItem {
    property var station : undefined
    property var provider: undefined

    id: index
    height: mainColumn.height

    Column {
        id: mainColumn
        width: parent.width
        spacing: Theme.paddingMedium

        Label {
            id: cityLabel
            text: station.cityName
            horizontalAlignment: Text.AlignRight
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeLarge
            anchors.right: parent.right
            truncationMode: TruncationMode.Fade
        }

        Label {
            id: streetLabel
            text: station.streetName
            horizontalAlignment: Text.AlignRight
            color: Theme.highlightColor
            anchors.right: parent.right
            truncationMode: TruncationMode.Fade
        }

        Item {
            id: row
            width: parent.width
            height: Math.max(image.height, dataColumn.height)
            anchors.margins: Theme.horizontalPageMargin
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                id: image
                height: 256
                source: "qrc:///Graphics/weatherIcons/256/" + station.stationIndex.id + ".png"
                anchors.centerIn: parent
            }

            Column {
                id: dataColumn
                anchors.right: parent.right
                anchors.bottom: image.bottom

                Label {
                    id: distanceLabel
                    text: station.distance + " km"
                    horizontalAlignment: Text.AlignRight
                    color: Theme.secondaryColor
                    anchors.right: parent.right
                    visible: station.distance
                }

                Label {
                    id: date
                    text: station.stationIndex.date
                    horizontalAlignment: Text.AlignRight
                    color: Theme.secondaryColor
                    anchors.right: parent.right
                }

                Label {
                    id: indexVersion
                    text: station.stationIndex.calculationModeName
                    horizontalAlignment: Text.AlignRight
                    color: Theme.secondaryColor
                    anchors.right: parent.right
                }
            }
        }

        Label {
            id: label

            text: station.stationIndex.name
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: Theme.fontSizeLarge
        }
    }
}
