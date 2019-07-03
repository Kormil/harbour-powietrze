#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QGuiApplication>
#include <sailfishapp.h>

#include "settings.h"
#include "modelsmanager.h"
#include "gpsmodule.h"

int main(int argc, char *argv[])
{
    // SailfishApp::main() will display "qml/harbour-powietrze.qml", if you need more
    // control over initialization, you can use:
    //
    //   - SailfishApp::application(int, char *[]) to get the QGuiApplication *
    //   - SailfishApp::createView() to get a new QQuickView * instance
    //   - SailfishApp::pathTo(QString) to get a QUrl to a resource file
    //   - SailfishApp::pathToMainQml() to get a QUrl to the main QML file
    //
    // To display the view, call "show()" (will show fullscreen on device).

    QGuiApplication * app = SailfishApp::application(argc, argv);
    QQuickView * view = SailfishApp::createView();

    ModelsManager modelsManager;
    modelsManager.createModels();
    modelsManager.bindToQml(view);

    GPSModule::bindToQml(view);
    Settings::bindToQml();

    modelsManager.loadSettings();

    view->setSource(SailfishApp::pathToMainQml());
    view->show();
    return app->exec();
}
