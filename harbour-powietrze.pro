# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-powietrze

QT += dbus positioning
CONFIG += sailfishapp

PKGCONFIG += \
    nemonotifications-qt5

SOURCES += src/harbour-powietrze.cpp \
    Types/stationlist.cpp \
    Models/stationlistmodel.cpp \
    Models/provincelistmodel.cpp \
    Types/provincelist.cpp \
    src/modelsmanager.cpp \
    src/connection.cpp \
    Types/station.cpp \
    Types/sensorlist.cpp \
    Models/sensorlistmodel.cpp \
    src/settings.cpp \
    Types/stationdata.cpp \
    src/gpsmodule.cpp \
    src/utils.cpp

DISTFILES += \
    qml/cover/CoverPage.qml \
    rpm/harbour-powietrze.changes.in \
    rpm/harbour-powietrze.changes.run.in \
    rpm/harbour-powietrze.spec \
    rpm/harbour-powietrze.yaml \
    translations/*.ts \
    qml/pages/FirstPage.qml \
    qml/pages/SelectStationPage.qml \
    qml/pages/SelectProvincePage.qml \
    qml/pages/sensorListPage.qml \
    qml/pages/SettingsPage.qml \
    qml/harbour-powietrze.qml \
    harbour-powietrze.desktop \
    qml/cover/StationCoverPage.qml \
    qml/pages/AboutPage.qml \
    qml/pages/LicensePage.qml \
    qml/pages/FindNearestPage.qml \
    qml/items/StationItem.qml \
    qml/items/StationFavouriteItem.qml \
    qml/items/StationNearestItem.qml

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-powietrze-pl.ts

HEADERS += \
    Types/stationlist.h \
    Models/stationlistmodel.h \
    Models/provincelistmodel.h \
    Types/provincelist.h \
    src/modelsmanager.h \
    src/connection.h \
    Types/station.h \
    Types/sensorlist.h \
    Models/sensorlistmodel.h \
    src/settings.h \
    Types/stationdata.h \
    src/gpsmodule.h \
    src/utils.h

RESOURCES += \
    graphics.qrc


license.files = LICENSE
license.path = /usr/share/$${TARGET}
INSTALLS += license

OTHER_FILES += nmea/*
nmea.files = nmea/*.nmea
nmea.path = /usr/share/$$TARGET/nmea
INSTALLS += nmea
