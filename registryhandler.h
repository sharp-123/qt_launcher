#ifndef REGISTRYHANDLER_H
#define REGISTRYHANDLER_H

#include <QObject>
#include <QSettings>
#include <windows.h>
#include <QDebug>


class RegistryHandler : public QObject
{
    Q_OBJECT
public:
    explicit RegistryHandler(QObject *parent = nullptr);

    Q_INVOKABLE QString readRegistryValue(const QString &keyName, const QString &valueName);
    Q_INVOKABLE bool writeRegistryValue(const QString &keyName, const QString &valueName, const QString &value);

private:
    QSettings settings;
signals:

public slots:
};

#endif // REGISTRYHANDLER_H
