import QtQuick 2.0
import Sailfish.Silica 1.0
import StationListModel 1.0

Page {
    id: page

    Component.onCompleted: {
        if (stationListProxyModel.rowCount() === 0) {
            hintLabel.visible = true
            hint.visible = true
            hint.start()
        }
    }

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {
            MenuItem {
                text: qsTr("About Powietrze")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
                }
            }

            MenuItem {
                text: qsTr("Settings")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
                }
            }

            MenuItem {
                text: qsTr("Select station")
                onClicked: {
                    stationListModel.requestStationListData()
                    pageStack.push(Qt.resolvedUrl("SelectProvincePage.qml"))
                }
            }
        }

//        PageHeader {
//            id: lastViewSection
//            title: qsTr("Last view")
//        }

//        BackgroundItem {
//            id: lastViewItem
//            height: lastViewNameLabel.height + lastViewValueLabel.height
//            anchors.top: lastViewSection.bottom
//            visible: false

//            Label {
//                id: lastViewNameLabel
//                x: Theme.horizontalPageMargin
//                text: stationListModel.selectedStation.name
//                color: lastViewItem.highlighted ? Theme.highlightColor : Theme.primaryColor
//                width: lastViewItem.width
//                truncationMode: TruncationMode.Fade
//            }

//            Label {
//                id: lastViewValueLabel
//                x: Theme.horizontalPageMargin
//                anchors.top: lastViewNameLabel.bottom
//                text: "Dobry"
//                font.pixelSize: Theme.fontSizeExtraSmall
//                color: lastViewItem.highlighted ? Theme.highlightColor : Theme.secondaryColor
//            }

//            onClicked: {
//                pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"))
//                stationListProxyModel.onItemClicked(index)
//            }
//        }

        PageHeader {
            id: favouriteSection
            title: qsTr("Favourite")
        }

       SilicaListView {
            id: listView
            width: parent.width
            height: parent.height
            anchors.top: favouriteSection.bottom
            spacing: Theme.paddingLarge


            model: StationListProxyModel {
                id: stationListProxyModel
                favourites: true
                stationModel: stationListModel
            }

            delegate: BackgroundItem {
                id: delegate
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

                onClicked: {
                    pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"))
                    stationListProxyModel.onItemClicked(index)
                }
            }
            VerticalScrollDecorator {}
        }

       ProgressBar {
           width: parent.width
           indeterminate: true
           id: loading
           anchors.bottom: parent.bottom
           enabled: false
           visible: false
       }

    }

    Connections {
        target: stationListModel
        onDataChanged: {
            stationListProxyModel.invalidate()

            if (stationListProxyModel.rowCount() === 0) {
                hintLabel.visible = true
                hint.visible = true
                hint.start()
            } else {
                hintLabel.visible = false
                hint.visible = false
                hint.stop()
            }
        }
    }

    Connections {
        target: stationListModel
        onFavourtiesUpdatingStarted: {
            loading.enabled = true
            loading.visible = true
        }
    }

    Connections {
        target: stationListModel
        onFavourtiesUpdated: {
            loading.enabled = false
            loading.visible = false
        }
    }

    InteractionHintLabel {
        id: hintLabel
        visible: false
        anchors.bottom: parent.bottom
        Behavior on opacity { FadeAnimation {} }
        text: qsTr("Pull down to select sations")
        anchors.fill: parent
    }

    TouchInteractionHint {
        id: hint
        visible: false
        loops: Animation.Infinite
        interactionMode: TouchInteraction.Pull
        direction: TouchInteraction.Down
    }
}
