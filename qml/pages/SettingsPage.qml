import QtQuick 2.0
import Sailfish.Silica 1.0
import Settings 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    SilicaFlickable{
        anchors.fill: parent

        Column
        {

        PageHeader {
            title: qsTr("Settings")
        }

            ComboBox {
                width: page.width
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


        SectionHeader {
            id: favouriteSection
            text: qsTr("GPS")
        }

            ComboBox {
                width: page.width
                label: qsTr("Update frequency")
                currentIndex: Settings.gpsUpdateFrequency

                menu: ContextMenu {
                    MenuItem { text: qsTr("Never") }
                    MenuItem { text: qsTr("30 minutes") }
                    MenuItem { text: qsTr("1 hour") }
                    MenuItem { text: qsTr("2 hours") }
                    MenuItem { text: qsTr("4 hours") }
                }

                onCurrentIndexChanged: {
                    Settings.gpsUpdateFrequency = currentIndex
                }
            }
}
    }
}
