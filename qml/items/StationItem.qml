import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    id: item

    Label {
        x: Theme.horizontalPageMargin
        text: model.description
        anchors.verticalCenter: parent.verticalCenter
        color: item.highlighted ? Theme.highlightColor : Theme.primaryColor
    }
}
