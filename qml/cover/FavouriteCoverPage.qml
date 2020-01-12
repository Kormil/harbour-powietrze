import QtQuick 2.0
import Sailfish.Silica 1.0

import StationListModel 1.0

CoverBackground {
    id: cover

    SilicaListView {
         id: listView
         width: parent.width
         height: cover.height - coverActionArea.height
         spacing: Theme.paddingMedium

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
                 anchors.left: parent.left
                 anchors.right: parent.right
                 anchors.leftMargin: Theme.horizontalPageMargin
                 text: model.description
                 color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                 width: delegate.width
                 truncationMode: TruncationMode.Fade
             }

             Label {
                 id: valueLabel
                 anchors.left: parent.left
                 anchors.leftMargin: Theme.horizontalPageMargin
                 anchors.top: nameLabel.bottom
                 text: model.indexName
                 font.pixelSize: Theme.fontSizeExtraSmall
                 color: delegate.highlighted ? Theme.highlightColor : Theme.secondaryColor
             }
         }
         VerticalScrollDecorator {}
     }

    CoverActionList {
        id: coverActions
        enabled: true
        iconBackground: true

        CoverAction {
            iconSource: "image://theme/icon-cover-favorite"
            onTriggered: {
                changeCoverPage(Qt.resolvedUrl("../cover/NearestCoverPage.qml"));
            }
        }
    }
}
