import QtQuick 2.0
import Sailfish.Silica 1.0
import Settings 1.0

BackgroundItem {
    property string stationName
    property var sensor: undefined
    property var stopAnimation: undefined
    property double properHeight: height
    property bool detailsVisible: dateRow.visible

    anchors.margins: Theme.horizontalPageMargin

    id: item
    height: values.height

    function nothing() {}

    function canExpand() {
        return sensor.date.length !== 0
    }

    function changeDetailsVisible() {
        dateRow.visible = !detailsVisible;
        unitRow.visible  = !detailsVisible;

        if (parseInt(sensor.norm) > -1) {
            normRow.visible = !detailsVisible;
        }
    }

    function expandDetails() {
        if (!dateRow.visible) {
            properHeight = values.height + dateRow.height
            if (parseInt(sensor.norm) > -1) {
                properHeight = properHeight + normRow.height
            }
            properHeight = properHeight + unitRow.height
        } else {
            properHeight = values.height
        }

        detailsVisible = dateRow.visible
        dateRow.visible = false
        normRow.visible = false
        unitRow.visible = false
    }

    Component.onCompleted: {
        if (Settings.expandValues && canExpand()) {
            expandDetails()
            changeDetailsVisible()
            height = properHeight
        }
    }

    ContextMenu {
        id: contextMenu
        MenuItem {
            id: copyMenuButton
            text: qsTr("Copy")
            onClicked: {
                var text = ""
                if (Settings.copyWithName) {
                    text = text + stationName + " ["
                }

                text = text + sensor.name + ": " + sensor.value

                if (dateRow.visible) {
                    text = text + ", " + dateLabel.text + ":" + sensor.date
                    text = text + ", " + normLabel.text + ":" + sensor.norm
                    text = text + ", " + unitLabel.text + ":" + sensor.unit
                }

                if (Settings.copyWithName) {
                    text = text + "]"
                }

                Clipboard.text = text
            }
        }
    }
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
                wrapMode: Text.Wrap
            }
            Label {
                horizontalAlignment: Text.AlignLeft
                text: sensor.value
                fontSizeMode: Theme.itemSizeExtraSmall
                color: Theme.highlightColor
            }
        }

        Row {
            id: dateRow
            visible: false
            spacing: Theme.paddingMedium
            Label {
                id: dateLabel
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
            id: normRow
            visible: false
            spacing: Theme.paddingMedium
            Label {
                id: normLabel
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
            id: unitRow
            visible: false
            spacing: Theme.paddingMedium
            Label {
                id: unitLabel
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
        if (animation.running) {
            return
        }

        if (!canExpand()) {
            return
        }

        expandDetails()

        stopAnimation = changeDetailsVisible
        animation.running = true
    }

    onPressAndHold: {
        if (animation.running) {
            return
        }

        properHeight = properHeight + copyMenuButton.height

        stopAnimation = nothing
        animation.running = true
        if (!contextMenu.active) {
            contextMenu.open(item)
        }
    }

    onReleased: {
        if (contextMenu.active) {
            animation.running = false

            properHeight = properHeight - copyMenuButton.height
            stopAnimation = nothing

            contextMenu.close()
            animation.running = true
        }
    }

    PropertyAnimation {
        id: animation;
        target: item;
        property: "height";
        to: properHeight;
        duration: 200
        easing.type: Easing.InOutQuad

        onStarted: {
        }

        onStopped: {
            stopAnimation()
        }
    }
}
