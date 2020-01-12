import QtQuick 2.0
import Sailfish.Silica 1.0

import StationListModel 1.0
import "../items"

CoverBackground {
    StationInfoCoverItem {
        station: stationListModel.selectedStation
    }
}

