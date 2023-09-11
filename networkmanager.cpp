#include "networkmanager.h"
#include "jsonmanager.h"


NetworkManager::NetworkManager(QObject *parent) : QObject(parent) {

    manager = new QNetworkAccessManager();
    assetManager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished, this, &NetworkManager::onLists);
    connect(assetManager, &QNetworkAccessManager::finished,this,&NetworkManager::onReply);
    connect(this, SIGNAL(beginDownload()),this,SLOT(onBeginRequest()));
    url_windows=QUrl("https://dev2.omochim.com/api/checkAppUpdate?os=1");
    url_mac=QUrl("https://dev2.omochim.com/api/checkAppUpdate?os=2");
}

void NetworkManager::onBeginRequest()
{
    if(!m_downloadQueue.isEmpty())
    {
        QNetworkRequest request = m_downloadQueue.dequeue();

        QString filePath = QCoreApplication::applicationDirPath() + "/app/"+request.url().toString().replace(baseUrl,"");
        QFileInfo fileInfo(filePath);
        QDir dir(fileInfo.absolutePath());

        if (!dir.exists()) {
            if (dir.mkpath(fileInfo.absolutePath())) {
    //             qDebug() << "Directory structure created: " << fileInfo.absolutePath();
            } else {
    //                qDebug() << "Failed to create directory structure: " << fileInfo.absolutePath();
            }
        }

        m_file = new QFile(filePath);

        if(!m_file->open(QIODevice::WriteOnly))
        {
            delete m_file;
            m_file = nullptr;
        }

        m_currentReply=assetManager->get(request);

        connect(m_currentReply, &QNetworkReply::readyRead, this, onReadyRead);
    }
    else
    {
        launchApp();

    }


}

void NetworkManager::onReadyRead()
{
    if(m_file)
    {
        m_file->write(m_currentReply->readAll());
    }
}

void NetworkManager::onReply(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NoError)
        {
            // save file
            m_file->flush();
            m_file->close();

            m_downloadedFileCount++;
            float percent =(float) m_downloadedFileCount/(float)m_totalFileCount;
            emit downloadedFileCountChanged(percent, "");
            delete m_file;
            m_file = nullptr;
            delete m_currentReply;
            m_currentReply = nullptr;
            emit beginDownload();
        }
        else
        {
            // Or delete it in case of error
            m_file->remove();
        }


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
            else
            {
                emit downloadedFileCountChanged(1, "");
                launchApp();
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

void NetworkManager::launchApp()
{

    QString program = QCoreApplication::applicationDirPath() + "/app/OmochimApp.exe";
    QStringList arguments;
    arguments << "/c" << "runas /user:Administrator" ;


    if (QProcess::startDetached(program, arguments)) {
        // The external app started successfully.
        // You can exit the original app if needed.
        QCoreApplication::quit();
    } else {
        DWORD errorCode = GetLastError();
        LPVOID errorMessageBuffer;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            errorCode,
            0, // Default language
            (LPWSTR)&errorMessageBuffer,
            0,
            NULL
        );

        QString errorMessage = QString::fromWCharArray((LPWSTR)errorMessageBuffer);
        LocalFree(errorMessageBuffer);
        emit error(errorMessage);
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
    QString base = jdoc->baseUrl;
    m_totalFileCount = jdoc->files.size();

    for (int i = 0; i < jdoc->files.size(); i++)
    {
        QJsonObject jsonObject = jdoc->files.at(i).toObject();
        QUrl reqUrl(base + jsonObject.value("url").toString());
        QNetworkRequest areq;
        areq.setUrl(reqUrl);
        m_downloadQueue.enqueue(areq);
    }

    emit beginDownload();
}


void NetworkManager::onLists(QNetworkReply *reply)
{
    // If an error occurs in the process of obtaining data
    if(reply->error()){
        emit error(reply->errorString());
    } else {
//        qDebug()<<reply->readAll();
        downloadedData = reply->readAll();
        QString filePath = QCoreApplication::applicationDirPath() + "/list.json";

        checkDownloadFile(filePath);
    }
    delete reply;
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
//             qDebug() << "Directory structure created: " << fileInfo.absolutePath();
            } else {
//                qDebug() << "Failed to create directory structure: " << fileInfo.absolutePath();
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
            delete source;
        }
        else
        {
             qDebug() << "Failed to create or save asset file: " << file.errorString();
        }
    }
}

