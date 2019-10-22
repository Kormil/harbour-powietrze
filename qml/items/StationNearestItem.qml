import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    property string name: ""
    property string distance: ""
    property string airQualityIndex: ""

    id: delegate
    contentHeight: Theme.itemSizeSmall
    height: nameLabel.height + valueLabel.height

    Label {
        id: nameLabel
        text: name
        anchors.left: parent.left
        anchors.leftMargin: Theme.horizontalPageMargin
        anchors.right: distanceLabel.left
        color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
        truncationMode: TruncationMode.Fade
    }

    Label {
        id: distanceLabel
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: Theme.horizontalPageMargin
        anchors.leftMargin: Theme.horizontalPageMargin
        text: distance
        font.pixelSize: Theme.fontSizeExtraSmall
        color: delegate.highlighted ? Theme.highlightColor : Theme.highlightColor
    }

    Label {
        id: valueLabel
        x: Theme.horizontalPageMargin
        anchors.top: nameLabel.bottom
        text: airQualityIndex
        font.pixelSize: Theme.fontSizeExtraSmall
        color: delegate.highlighted ? Theme.highlightColor : Theme.secondaryColor
    }
}
