import QtQuick 2.0
import Sailfish.Silica 1.0
import StationListModel 1.0
import GPSModule 1.0
import Settings 1.0
import Nemo.Notifications 1.0
import Nemo.DBus 2.0

import ProviderListModel 1.0

import "../items"

Page {
    DBusAdaptor {
        service: "harbour.powietrze.service"
        path: "/harbour/powietrze/service"
        iface: "harbour.powietrze.service"

        function openPage(item) {
            app.activate();
            pageStack.pop(page, PageStackAction.Immediate);
            pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"), {}, PageStackAction.Immediate)
            stationListModel.onItemClicked(item)
        }
    }

    property bool nearestStationEnabled: Settings.gpsUpdateFrequency
    property int selectedTimeBreak
    id: page

    Component.onCompleted: {
        if (stationListProxyModel.rowCount() === 0) {
            hintLabel.visible = true
            hint.visible = true
            hint.start()
        }

        if (nearestStationEnabled)
            stationListModel.findNearestStation()
    }

    ProviderListProxyModel {
        id: providerListProxyModel
        providerModel: providerListModel
        enabledFilter: true
    }

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        id: mainItem
        anchors.fill: parent
        contentHeight: listView.height + nearestStation.height + favouriteSection.height

        VerticalScrollDecorator {}

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
                    if (providerListProxyModel.rowCount() === 1) {
                        providerListProxyModel.onItemClicked(0)
                        pageStack.push(Qt.resolvedUrl("SelectCountryPage.qml"))
                    } else {
                        pageStack.push(Qt.resolvedUrl("SelectProviderPage.qml"))
                    }
                    countryListModel.requestCountryList()
                }
            }
        }

        BackgroundItem {
            id: nearestHeader
            width: page.width
            height: nearestStation.height
            visible: nearestStationEnabled

            BusyIndicator {
                anchors.centerIn: parent
                running: true
                size: BusyIndicatorSize.Large
            }

            onClicked: {
                pageStack.push(stopLocatingDialog)
            }
        }

        StationInfoItem {
            id: nearestStation
            visible: nearestStationEnabled
            station: stationListModel.nearestStation

            onClicked: {
                if (gps.knowAnyPosition) {
                    pageStack.push(Qt.resolvedUrl("StationInfoPage.qml"))
                    stationListModel.onStationClicked(stationListModel.nearestStation)
                }
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
            anchors.top: favouriteSection.bottom
            width: parent.width
            height: contentHeight

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
                nearestStation.visible = false
                nearestStation.enabled = false
                nearestHeader.visible = false
            }
        }
    }

    Connections {
        target: gps
        onPositionRequested: {
            if (nearestStationEnabled)
            {
                nearestHeader.visible = true
                nearestStation.visible = false
                favouriteSection.anchors.top = nearestHeader.bottom
            }
        }
    }

    Connections {
        target: stationListModel
        onNearestStationFounded: {
            nearestHeader.visible = false
            nearestStation.visible = true
            nearestStation.enabled = true
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

    Component {
        id: stopLocatingDialog

        Dialog {
            canAccept: selector.currentIndex >= 0
            acceptDestination: page
            acceptDestinationAction: PageStackAction.Pop

            onAccepted: {
                if (selector.currentIndex == 0) {
                    gps.stopLocating();
                } else {
                    gps.pauseLocating(selector.currentItem.value)
                }
            }

            Flickable {
                width: parent.width
                height: parent.height
                interactive: false

                Column {
                    width: parent.width

                    DialogHeader {
                        title: qsTr("Stop locating")
                    }

                    ComboBox {
                        id: selector

                        width: parent.width
                        currentIndex: 0

                        menu: ContextMenu {
                            Repeater {
                                model: [ {name: qsTr('Only once'), value: 0},
                                    {name: qsTr('4 hours'), value: 4},
                                    {name: qsTr('8 hours'), value: 8},
                                    {name: qsTr('24 hours'), value: 24},
                                    {name: qsTr('Permanently'), value: -1}]

                                MenuItem {
                                    property int value: modelData.value
                                    text: modelData.name
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
