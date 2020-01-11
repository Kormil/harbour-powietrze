import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    id: delegate
    height: column.height + Theme.paddingLarge

    Column {
        id: column
        anchors.centerIn: parent

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
            text: model.indexName
            font.pixelSize: Theme.fontSizeExtraSmall
            color: delegate.highlighted ? Theme.highlightColor : Theme.secondaryColor
        }
    }
}
