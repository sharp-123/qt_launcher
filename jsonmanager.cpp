#include "jsonmanager.h"

JsonManager::JsonManager(QObject *parent) : QObject(parent)
{
    formatVersion="";
    version = "";
    baseUrl="";
//    files=[];
}

void JsonManager::parseDataFrom(QString data)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toUtf8());

    if (!jsonDoc.isObject()) {
        qDebug() << "Invalid JSON data.";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    if (jsonObj.contains("formatVersion")) {
        formatVersion = jsonObj["formatVersion"].toString();
    }

    if (jsonObj.contains("version")) {
        version = jsonObj["version"].toString();
    }

    if (jsonObj.contains("baseurl") ) {
        baseUrl = jsonObj["baseurl"].toString();
    }

    if (jsonObj.contains("files") && jsonObj["files"].isArray()) {
        files = jsonObj["files"].toArray();
    }
    else {
        qDebug() << "No 'files' array found in JSON data.";
    }
}
