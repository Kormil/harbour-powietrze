import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    property var provider : undefined
    id: delegate
    width: parent.width
    height: column.height

    Column {
        id: column
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Theme.paddingMedium

        Image {
            id: icon
            source: provider ? "qrc:///Graphics/providerIcons/" + provider.icon : ""
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            Label {
                text: "Name "
                color: Theme.secondaryHighlightColor
                horizontalAlignment: Text.AlignRight
                fontSizeMode: Theme.fontSizeSmall
            }
            Label {
                horizontalAlignment: Text.AlignLeft
                text: provider ? provider.name : ""
                fontSizeMode: Theme.fontSizeSmall
                color: Theme.highlightColor
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            Label {
                text: "Site "
                color: Theme.secondaryHighlightColor
                horizontalAlignment: Text.AlignRight
                fontSizeMode: Theme.fontSizeSmall
            }
            Label {
                horizontalAlignment: Text.AlignLeft
                text: provider ? provider.site : ""
                fontSizeMode: Theme.fontSizeSmall
                color: Theme.highlightColor
            }
        }
    }
}
