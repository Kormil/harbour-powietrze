import QtQuick 2.0
import Sailfish.Silica 1.0

import StationListModel 1.0

CoverBackground {

    SilicaListView {
         id: listView
         width: parent.width
         height: parent.height
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
}
