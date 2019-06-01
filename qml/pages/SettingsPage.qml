import QtQuick 2.0
import Sailfish.Silica 1.0
import Settings 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    SilicaListView {
        anchors.fill: parent

        header: PageHeader {
            title: qsTr("Settings")
        }

        model: VisualItemModel {
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
        }
    }
}
