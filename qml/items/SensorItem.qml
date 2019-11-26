import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    property var sensor: undefined
    property double properHeight: height
    anchors.margins: Theme.horizontalPageMargin

    id: item
    height: values.height

    Column {
        Row {
            id: values
            spacing: Theme.paddingMedium

            Label {
                width: page.width / 2
                text: sensor.name
                color: Theme.secondaryHighlightColor
                horizontalAlignment: Text.AlignRight
                fontSizeMode: Theme.itemSizeExtraSmall
            }
            Label {
                horizontalAlignment: Text.AlignLeft
                text: sensor.value
                fontSizeMode: Theme.itemSizeExtraSmall
                color: Theme.highlightColor
            }
        }

        Row {
            id: dateLabel
            visible: false
            spacing: Theme.paddingMedium
            Label {
                width: page.width / 2
                text: qsTr("date")
                color: Theme.secondaryColor
                horizontalAlignment: Text.AlignRight
                fontSizeMode: Theme.itemSizeExtraSmall
            }
            Label {
                horizontalAlignment: Text.AlignLeft
                text: sensor.date
                fontSizeMode: Theme.itemSizeExtraSmall
                color: Theme.primaryColor
            }
        }

        Row {
            id: normLabel
            visible: false
            spacing: Theme.paddingMedium
            Label {
                width: page.width / 2
                text: qsTr("norm")
                color: Theme.secondaryColor
                horizontalAlignment: Text.AlignRight
                fontSizeMode: Theme.itemSizeExtraSmall
            }
            Label {
                horizontalAlignment: Text.AlignLeft
                text: sensor.norm + "%"
                fontSizeMode: Theme.itemSizeExtraSmall
                color: Theme.primaryColor
            }
        }

        Row {
            id: unitLabel
            visible: false
            spacing: Theme.paddingMedium
            Label {
                width: page.width / 2
                text: qsTr("unit")
                color: Theme.secondaryColor
                horizontalAlignment: Text.AlignRight
                fontSizeMode: Theme.itemSizeExtraSmall
            }
            Label {
                horizontalAlignment: Text.AlignLeft
                text: sensor.unit
                fontSizeMode: Theme.itemSizeExtraSmall
                color: Theme.primaryColor
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.10) }
        }
    }

    onClicked: {
        if (!dateLabel.visible) {
            properHeight = values.height + dateLabel.height
            if (parseInt(sensor.norm) > -1) {
                properHeight = properHeight + normLabel.height
            }
            properHeight = properHeight + unitLabel.height
        } else {
            properHeight = values.height
        }
        animation.running = true
    }

    PropertyAnimation {
        property bool contentVisible: dateLabel.visible
        id: animation;
        target: item;
        property: "height";
        to: properHeight;
        duration: 150
        easing.type: Easing.InQuad

        onStarted: {
            contentVisible = dateLabel.visible;
            dateLabel.visible = false;
            normLabel.visible = false;
            unitLabel.visible = false;
        }

        onStopped: {
            dateLabel.visible = !contentVisible;
            unitLabel.visible  = !contentVisible;

            if (parseInt(sensor.norm) > -1) {
                normLabel.visible = !contentVisible;
            }
        }
    }
}
