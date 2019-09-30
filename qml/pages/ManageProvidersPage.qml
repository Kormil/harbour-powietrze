import QtQuick 2.0
import Sailfish.Silica 1.0

import StationListModel 1.0
import ProvinceListModel 1.0
import ProviderListModel 1.0

import "../items"

Page {

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView
        model: providerListModel

        anchors.fill: parent
        header: PageHeader {
            title: qsTr("Providers")
        }

        delegate: BackgroundItem {
            id: item
            height: labels.height + Theme.paddingMedium + Theme.paddingMedium

            onClicked: {
                providerListModel.onItemClicked(index)
                pageStack.push(Qt.resolvedUrl("../providers/" + providerListModel.selectedProvider.name + "Page.qml"))
            }

            Image {
                id: icon
                height: item.height - Theme.paddingMedium
                width: height
                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter

                source: "qrc:///Graphics/providerIcons/" + model.icon
             }

            Item {
                id: labels
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: icon.right
                anchors.leftMargin: Theme.paddingMedium

                height: nameLabel.height + enabledLabel.height

                Label {
                    id: nameLabel

                    anchors.verticalCenter: model.enabled ? parent.verticalCenter : undefined

                    text: model.name
                    color: model.enabled ? Theme.primaryColor : Theme.highlightColor
                    truncationMode: TruncationMode.Fade
                }

                Label {
                    id: enabledLabel

                    anchors.top: nameLabel.bottom

                    text: qsTr("Disabled")
                    visible: !model.enabled
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: item.highlighted ? Theme.highlightColor : Theme.secondaryColor
                }

                Connections {
                    target: providerListModel
                    onDataChanged: {
                        if (model.enabled) {
                            nameLabel.anchors.top = undefined
                            nameLabel.anchors.verticalCenter = labels.verticalCenter
                        } else {
                            nameLabel.anchors.verticalCenter = undefined
                            nameLabel.anchors.top = labels.top
                        }
                    }
                }
            }
        }
        VerticalScrollDecorator {}

    }
}
