#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "networkmanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;



    qmlRegisterType<NetworkManager>("MyApp", 1, 0, "NetworkManager");

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
