import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    id: delegate
    height: nameLabel.height + providerLabel.height

    Label {
        id: nameLabel
        text: model.description
        anchors.left: parent.left
        anchors.leftMargin: Theme.horizontalPageMargin
        anchors.right: distanceLabel.left
        color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
        truncationMode: TruncationMode.Fade
    }

    Label {
        id: distanceLabel
        anchors.right: parent.right
        anchors.rightMargin: Theme.horizontalPageMargin
        anchors.leftMargin: Theme.horizontalPageMargin
        text: model.distance + " km"
        font.pixelSize: Theme.fontSizeExtraSmall
        color: delegate.highlighted ? Theme.highlightColor : Theme.highlightColor
    }

    Label {
        id: providerLabel
        anchors.right: parent.right
        anchors.top: nameLabel.bottom
        anchors.rightMargin: Theme.horizontalPageMargin
        anchors.leftMargin: Theme.horizontalPageMargin
        text: model.providerName
        font.pixelSize: Theme.fontSizeExtraSmall
        color: delegate.highlighted ? Theme.highlightColor : Theme.secondaryColor
    }
}
