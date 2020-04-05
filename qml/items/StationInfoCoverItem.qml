import QtQuick 2.2
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
        width: parent.width - 2 * Theme.horizontalPageMargin
    }

    Label {
        id: streetName
        anchors.top: pageHeader.bottom
        text: station.streetName
        font.pixelSize: Theme.fontSizeExtraSmall
        color: Theme.primaryColor
        x: Theme.horizontalPageMargin
        truncationMode: TruncationMode.Fade
        width: parent.width - 2 * Theme.horizontalPageMargin
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
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: Theme.paddingSmall
            anchors.top: image.bottom
            width: parent.width
            truncationMode: TruncationMode.Fade

            onTextChanged: {
                if (station.stationIndex.nameWidth(label.font) < label.width  - 2 * Theme.horizontalPageMargin) {
                    label.horizontalAlignment = Text.AlignHCenter
                } else {
                    label.horizontalAlignment = Text.AlignLeft
                }
            }
        }
    }
}
