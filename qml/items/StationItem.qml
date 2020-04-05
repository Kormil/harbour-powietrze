import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    id: item
    width: parent.width

    Label {
        width: parent.width - Theme.horizontalPageMargin * 2
        x: Theme.horizontalPageMargin
        text: model.description
        anchors.verticalCenter: parent.verticalCenter
        color: item.highlighted ? Theme.highlightColor : Theme.primaryColor
        truncationMode: TruncationMode.Fade
    }
}
