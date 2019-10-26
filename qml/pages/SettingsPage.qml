import QtQuick 2.0
import Sailfish.Silica 1.0
import Settings 1.0
import GPSModule 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    SilicaFlickable{
        anchors.fill: parent
        anchors.leftMargin: Theme.paddingMedium
        anchors.rightMargin: Theme.paddingMedium
        contentHeight: column.height

        VerticalScrollDecorator {}

        Column
        {
            id: column
            width: parent.width

            PageHeader {
                title: qsTr("Settings")
            }

            ComboBox {
                width: parent.width
                label: qsTr("Units type")
                currentIndex: Settings.unitType

                menu: ContextMenu {
                    MenuItem { text: qsTr("Microgram") }
                    MenuItem { text: qsTr("Milligram") }
                }

                onCurrentIndexChanged: {
                    Settings.unitType = currentIndex
                }
            }

            TextSwitch {
                text: qsTr("Notifications")
                checked: Settings.notifications

                onCheckedChanged: {
                    if (Settings.notifications !== checked) {
                        Settings.notifications = checked
                    }
                }
            }

            SectionHeader {
                id: favouriteSection
                text: qsTr("GPS")
            }

            TextSwitch {
                text: qsTr("Paused")
                checked: gps.paused
                visible: gps.paused

                onCheckedChanged: {
                    if (checked != gps.paused) {
                        gps.paused = checked
                    }
                }
            }

            ComboBox {
                width: parent.width
                label: qsTr("Update frequency")
                currentIndex: Settings.gpsUpdateFrequency

                menu: ContextMenu {
                    MenuItem { text: qsTr("Never") }
                    MenuItem { text: qsTr("30 minutes") }
                    MenuItem { text: qsTr("1 hour") }
                    MenuItem { text: qsTr("2 hours") }
                    MenuItem { text: qsTr("4 hours") }
                }

                description: qsTr("Works only with the screen on. If the screen is off, the program will automatically look for the nearest station after turning the screen on.")

                onCurrentIndexChanged: {
                    Settings.gpsUpdateFrequency = currentIndex
                }
            }

            BackgroundItem {
                width: parent.width

                Image {
                    id: image

                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter

                    source: "image://theme/icon-m-right?" + (label.highlighted
                                                             ? Theme.highlightColor
                                                             : Theme.primaryColor)
                }

                Label {
                    id: label

                    anchors.right: image.left
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: Theme.horizontalPageMargin
                    anchors.rightMargin: Theme.paddingMedium

                    horizontalAlignment: Text.AlignRight
                    truncationMode: TruncationMode.Fade
                    text: qsTr("Manage providers")
                    color: highlighted ? Theme.highlightColor : Theme.primaryColor
                }

                Rectangle {
                    anchors.fill: parent
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.15) }
                        GradientStop { position: 1.0; color: "transparent" }
                    }
                }

                onClicked: {
                    pageStack.push(Qt.resolvedUrl("ManageProvidersPage.qml"))
                }
            }
        }
    }
}
