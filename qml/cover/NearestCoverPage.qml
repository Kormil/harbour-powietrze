import QtQuick 2.0
import Sailfish.Silica 1.0

import StationListModel 1.0
import "../items"

CoverBackground {
    id: cover

    StationInfoCoverItem {
        station: stationListModel.nearestStation
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: stationListModel.nearestStation === null
        size: BusyIndicatorSize.Large
    }

    CoverActionList {
        id: coverActions
        enabled: true

        CoverAction {
            iconSource: "image://theme/icon-m-whereami"
            onTriggered: {
                changeCoverPage(Qt.resolvedUrl("../cover/FavouriteCoverPage.qml"));
            }
        }
    }
}

