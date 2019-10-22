import QtQuick 2.0
import Sailfish.Silica 1.0
import Settings 1.0

import ProviderListModel 1.0
import "../items"

Page {
    property var provider: providerListModel.selectedProvider
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.All

    SilicaFlickable{
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("License")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("../pages/ProviderLicensePage.qml"))
                }
            }
        }

        Column
        {
            anchors.fill: parent

            PageHeader {
                id: header
                title: provider ? provider.name : ""
            }

            ProviderItem {
                provider: page.provider
            }

            TextSwitch {
                id: enabledTextSwitch
                text: qsTr("Enabled")
                checked: provider ? provider.enabled : false

                onCheckedChanged: {
                    if (provider && provider.enabled !== checked) {
                        if (checked && apiKeyTextField.text.length <= 0) {
                            pageStack.push(enableDialog)
                        } else {
                            provider.enabled = checked
                        }
                    }
                }
            }

            TextField {
                id: apiKeyTextField
                text: provider ? provider.apiKey : ""
                label: "Api key"
                placeholderText: label
                width: parent.width
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: passwordField.focus = true

                onTextChanged: {
                    if (provider) {
                        provider.apiKey = text
                    }
                }
            }

            ComboBox {
                width: page.width
                label: qsTr("Air quality index")
                currentIndex: provider.airQualityIndex

                menu: ContextMenu {
                    MenuItem { text: "The Common Air Quality Index" }
                    MenuItem { text: "European air quality index" }
                }

                onCurrentIndexChanged: {
                    if (provider && provider.airQualityIndex !== currentIndex) {
                        provider.airQualityIndex = currentIndex
                    }
                }
            }
        }
    }

    Component {
        id: enableDialog

        Dialog {
            canAccept: textFieldDialog.text.length > 0
            acceptDestination: page
            acceptDestinationAction: PageStackAction.Pop

            onAccepted: {
                enabledTextSwitch.checked = true
                provider.enabled = true
                provider.apiKey = textFieldDialog.text
            }

            onRejected: {
                enabledTextSwitch.checked = false
            }

            Flickable {
                width: parent.width
                height: parent.height
                interactive: false

                Column {
                    width: parent.width
                    height: parent.height

                    DialogHeader {
                        title: qsTr("Api key for Airly")
                    }

                    LinkedLabel {
                        x: Theme.horizontalPageMargin
                        plainText: qsTr("To get access to Airly api you must generate key. You can get your own api key from\nhttps://developer.airly.eu")
                        color: Theme.primaryColor
                        wrapMode: Text.Wrap
                        width: parent.width - 2 * Theme.horizontalPageMargin
                        font.pixelSize: Theme.fontSizeSmall
                    }

                    TextField {
                        id: textFieldDialog
                        focus: true
                        label: "Api key"
                        placeholderText: label
                        width: parent.width
                        EnterKey.iconSource: "image://theme/icon-m-enter-next"
                        EnterKey.onClicked: passwordField.focus = true
                    }
                }
            }
        }
    }
}
