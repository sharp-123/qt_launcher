#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "networkmanager.h"
#include "registryhandler.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;



    qmlRegisterType<NetworkManager>("MyApp", 1, 0, "NetworkManager");
    qmlRegisterType<RegistryHandler>("Registry", 1, 0, "RegistryHandler");

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
