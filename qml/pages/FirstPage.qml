import QtQuick 2.0
import Sailfish.Silica 1.0
import StationListModel 1.0
import GPSModule 1.0
import Settings 1.0

import "../items"

Page {
    property bool nearestStationEnabled: Settings.gpsUpdateFrequency
    id: page

    Component.onCompleted: {
        if (stationListProxyModel.rowCount() === 0) {
            hintLabel.visible = true
            hint.visible = true
            hint.start()
        }

        if (nearestStationEnabled)
            gps.requestPosition();
    }

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        id: mainItem
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

        SectionHeader {
            id: nearestHeader
            anchors.left: parent.left
            width: parent.width - nearestStationBusy.widthWithMargins
            text: qsTr("Nearest station")
            font.pixelSize: Theme.fontSizeLarge
            visible: nearestStationEnabled
        }

        BusyIndicator {
            property int widthWithMargins: nearestStationBusy.width + nearestStationBusy.anchors.leftMargin + nearestStationBusy.anchors.rightMargin

            id: nearestStationBusy
            anchors.verticalCenter: nearestHeader.verticalCenter
            anchors.left: nearestHeader.right
            anchors.leftMargin: Theme.paddingSmall
            anchors.rightMargin: Theme.paddingSmall
            running: true
            size: BusyIndicatorSize.Small
            visible: nearestStationEnabled
        }

        StationNearestItem {
            id: nearestStation
            anchors.top: nearestHeader.bottom
            visible: nearestStationEnabled

            onClicked: {
                pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"))
                stationListModel.onStationClicked(stationListModel.nearestStation)
            }
        }

        SectionHeader {
            id: favouriteSection
            anchors.top: nearestStationEnabled ? nearestHeader.bottom : parent.top
            text: qsTr("Favourite")
            font.pixelSize: Theme.fontSizeLarge
        }

        SilicaListView {
            id: listView
            spacing: Theme.paddingLarge
            anchors.topMargin: Theme.paddingLarge
            anchors.top: favouriteSection.bottom
            width: parent.width
            height: parent.height

            model: StationListProxyModel {
                id: stationListProxyModel
                favourites: true
                stationModel: stationListModel
            }

            delegate: StationFavouriteItem {
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"))
                    stationListProxyModel.onItemClicked(index)
                }
            }
            VerticalScrollDecorator {}
        }
    }

    ProgressBar {
        width: parent.width
        indeterminate: true
        id: loading
        anchors.bottom: parent.bottom
        enabled: false
        visible: false
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

    Connections {
        target: Settings
        onGpsUpdateFrequencyChanged: {
            if (!Settings.gpsUpdateFrequency)
            {
                favouriteSection.anchors.top = favouriteSection.parent.top
            }
        }
    }

    Connections {
        target: gps
        onPositionRequested: {
            if (nearestStationEnabled)
            {
                nearestStationBusy.visible = true
                nearestHeader.width = page.width - nearestStationBusy.widthWithMargins
                favouriteSection.anchors.top = nearestHeader.bottom
            }
        }
    }

    Connections {
        target: stationListModel
        onNearestStationFounded: {
            nearestStationBusy.visible = false
            nearestHeader.width = page.width - Theme.horizontalPageMargin

            if (nearestStationEnabled)
            {
                nearestStation.name = stationListModel.nearestStation.name
                nearestStation.distance = stationListModel.nearestStation.distance + " km"
                nearestIndexChangedConnetion.target = stationListModel.nearestStation

                favouriteSection.anchors.top = nearestStation.bottom
            }
        }
    }

    Connections {
        id: nearestIndexChangedConnetion
        target: stationListModel.nearestStation
        onStationIndexChanged: {
            nearestStation.index = stationListModel.nearestStation.stationIndex.name
        }
    }

    InteractionHintLabel {
        id: hintLabel
        visible: false
        anchors.bottom: parent.bottom
        Behavior on opacity { FadeAnimation {} }
        text: qsTr("Pull down to select stations")
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
