import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    id: delegate
    anchors.topMargin: Theme.paddingMedium
    anchors.bottomMargin: anchors.topMargin
    height: nameLabel.height + valueLabel.height

    Label {
        id: nameLabel
        x: Theme.horizontalPageMargin
        text: model.description
        color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
        width: delegate.width
        truncationMode: TruncationMode.Fade
    }

    Label {
        id: valueLabel
        x: Theme.horizontalPageMargin
        anchors.top: nameLabel.bottom
        text: model.indexName
        font.pixelSize: Theme.fontSizeExtraSmall
        color: delegate.highlighted ? Theme.highlightColor : Theme.secondaryColor
    }
}
