#include "registryhandler.h"

RegistryHandler::RegistryHandler(QObject *parent) : QObject(parent)
{

}

QString RegistryHandler::readRegistryValue(const QString &keyName, const QString &valueName) {
    QSettings settings(keyName, QSettings::NativeFormat);


//    QStringList allKeys = settings.allKeys();

//    for (const QString &key : allKeys) {
//        QVariant value = settings.value(key);
//        qDebug() << key << ": " << value.toString();
//    }

    QString ret=settings.value(valueName).toString();
    return ret;

}

bool RegistryHandler::writeRegistryValue(const QString &keyName, const QString &valueName, const QString &value) {
    QSettings settings(keyName, QSettings::NativeFormat);
    settings.setValue(valueName,value);
    return true; // You can add error handling if needed
}
