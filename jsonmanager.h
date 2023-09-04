#ifndef JSONMANAGER_H
#define JSONMANAGER_H

#include <QObject>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

class JsonManager : public QObject
{
    Q_OBJECT
public:
    QString formatVersion;
    QString version;
    QString baseUrl;
    QJsonArray files;

public:
    explicit JsonManager(QObject *parent = nullptr);
    void parseDataFrom(QString data);
};

#endif // JSONMANAGER_H
