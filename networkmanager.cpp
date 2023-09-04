#include "networkmanager.h"
#include "jsonmanager.h"


NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {

    manager = new QNetworkAccessManager();
    assetManager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished, this, &NetworkManager::onLists);
    connect(assetManager, &QNetworkAccessManager::finished,this,&NetworkManager::onAssets);
    url_windows=QUrl("https://dev2.omochim.com/api/checkAppUpdate?os=1");
    url_mac=QUrl("https://dev2.omochim.com/api/checkAppUpdate?os=2");
}


void NetworkManager::downloadListsFromServer()
{
    QNetworkRequest request;
    request.setUrl(url_windows);
    manager->get(request);
    emit loadingList();
}

bool NetworkManager::jsonObjectIsUpdated(const QJsonObject obj1, const QJsonObject obj2)
{
    if(obj1.value("hash").toString()==obj2.value("hash").toString())
        return false;
    else
        return true;
}

JsonManager* NetworkManager::newAndUpdatedObjects(JsonManager *fileDoc, JsonManager *downDoc)
{
    JsonManager* result = new JsonManager;
    QString fileID;
    QString downID;
    result->baseUrl=downDoc->baseUrl;
    result->version=downDoc->version;
    result->formatVersion=downDoc->formatVersion;
    for (int i = 0; i < downDoc->files.size(); ++i) {
        bool foundInOld = false;
        for (int j = 0; j < fileDoc->files.size(); ++j) {
            fileID = fileDoc->files.at(j).toObject().value("id").toString();
            downID = downDoc->files.at(i).toObject().value("id").toString();
            qDebug()<<fileDoc->files.at(j).toObject().value("id");
            if (fileID == downID) {
                if (jsonObjectIsUpdated(fileDoc->files.at(j).toObject(), downDoc->files.at(i).toObject())) {
                    result->files.append(downDoc->files.at(i).toObject());
                }
             foundInOld = true;
             break;
            }
        }
        if (!foundInOld) {
            result->files.append(downDoc->files.at(i).toObject());
        }
    }
    return result;
}

void NetworkManager::checkDownloadFile(QString path)
{
    JsonManager jdoc;
    JsonManager jdocFromFile;
    JsonManager* updateDoc;
    jdoc.parseDataFrom(downloadedData);
    baseUrl=jdoc.baseUrl;
    QFile file(path);
    if (QFile::exists(path))
    {
        qDebug() << "File already exists at:" << path;
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray fileData = file.readAll();
            file.close();

            jdocFromFile.parseDataFrom(fileData);

            if(jdoc.version != jdocFromFile.version)
            {
                emit versionNotify(jdoc.version);
                updateDoc=newAndUpdatedObjects(&jdocFromFile,&jdoc);
                downloadDataFromServer(updateDoc);
                if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    file.write(downloadedData); // Write new content
                    file.close(); // Close the file after writing

                    qDebug() << "Updated JSON data saved successfully at:" << path;
                } else {
                    qDebug() << "Failed to write updated JSON data:" << path;
                }
            }
        } else {
            qDebug() << "Failed to open file:" << path;
        }
    }
    else
    {
        qDebug() << "No such file exists at:" << path;
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(downloadedData);
            file.close();

            qDebug() << "JSON file created and saved successfully at:" << path;
            downloadDataFromServer(&jdoc);
            emit versionNotify(jdoc.version);

        }
        else
        {
            qDebug() << "Failed to create or save JSON file.";
        }
    }
}

int NetworkManager::downloadedFileCount() const {
    return m_downloadedFileCount;
}

void NetworkManager::initialize()
{
    downloadListsFromServer();
}

void NetworkManager::downloadDataFromServer(JsonManager *jdoc){
    QQueue<QNetworkRequest> downloadQueue;
    QString base = jdoc->baseUrl;
    m_totalFileCount = jdoc->files.size();

//    for (int i = 0; i < m_totalFileCount; i++) {
//        QJsonObject jsonObject = jdoc->files.at(i).toObject();
//        QUrl reqUrl(base + jsonObject.value("url").toString());
//        QNetworkRequest areq;
//        areq.setUrl(reqUrl);

//        QNetworkReply *reply = assetManager->get(areq);
//        qDebug()<<"requesting:"<<jsonObject.value("url").toString();
//        // Connect each reply's finished signal to a slot for handling it
//        connect(reply, &QNetworkReply::finished, this, &NetworkManager::onAssets);
//        connect(reply, &QNetworkReply::readyRead, this, &NetworkManager::onReadingFromServer);
//    }


    for (int i = 0; i < jdoc->files.size(); i++)
    {
        QJsonObject jsonObject = jdoc->files.at(i).toObject();
        QUrl reqUrl(base + jsonObject.value("url").toString());
        QNetworkRequest areq;
        areq.setUrl(reqUrl);
        downloadQueue.enqueue(areq);
    }

    while (!downloadQueue.isEmpty()) {

        QNetworkRequest request = downloadQueue.dequeue();
        assetManager->get(request);
    }
}


void NetworkManager::onLists(QNetworkReply *reply)
{
    // If an error occurs in the process of obtaining data
    if(reply->error()){
        emit error(reply->errorString());
    } else {
        downloadedData = reply->readAll();
        QString filePath = QCoreApplication::applicationDirPath() + "/list.json";

        checkDownloadFile(filePath);
    }
}



void NetworkManager::onAssets(QNetworkReply *reply)
{
    if (!reply) {
        // Handle the case where sender() did not return a valid reply
        return;
    }

    // If an error occurs in the process of obtaining data
    if(reply->error()){
        // We inform about it and show the error information
        qDebug() << "ERROR";
        qDebug() << reply->errorString();
        emit error(reply->errorString());
    } else {
        QString filePath = QCoreApplication::applicationDirPath() + "/app/"+reply->url().toString().replace(baseUrl,"");
        QFileInfo fileInfo(filePath);
        QDir dir(fileInfo.absolutePath());

        if (!dir.exists()) {
            if (dir.mkpath(fileInfo.absolutePath())) {
             qDebug() << "Directory structure created: " << fileInfo.absolutePath();
            } else {
                qDebug() << "Failed to create directory structure: " << fileInfo.absolutePath();
            }
        }
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly))
        {
            QIODevice* source = reply; // No readAll() here
            while (!source->atEnd()) {
                QByteArray buffer = source->read(8192); // Read a chunk
                file.write(buffer); // Write the chunk to the file
            }
//            file.write(reply->readAll());
            file.close();
            m_downloadedFileCount++;
            float percent =(float) m_downloadedFileCount/(float)m_totalFileCount;
            emit downloadedFileCountChanged(percent, filePath);
        }
        else
        {
             qDebug() << "Failed to create or save asset file: " << file.errorString();
        }
    }
    reply->deleteLater();
}

