#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H


#include <QObject>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <jsonmanager.h>
#include <QQueue>
#include <QProcess>

class NetworkManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int downloadedFileCount READ downloadedFileCount NOTIFY downloadedFileCountChanged)

private:
    int m_downloadedFileCount = 0;
    int m_totalFileCount = 0;
    QNetworkAccessManager *manager;
    QNetworkAccessManager *assetManager;
    QString baseUrl;
    QUrl url_windows;
    QUrl url_mac;
    bool jsonObjectIsUpdated(const QJsonObject obj1, const QJsonObject obj2);
public:
    Q_INVOKABLE void initialize();
    int downloadedFileCount() const;
    QList<QUrl> downloadingUrls;
    QByteArray downloadedData;
    explicit NetworkManager(QObject *parent = nullptr);
    void checkDownloadFile(QString path); //initialize list.json file
    void downloadListsFromServer(); //request for downloading list
    void downloadDataFromServer(JsonManager *jdoc);
    JsonManager* newAndUpdatedObjects(JsonManager *fileDoc, JsonManager *downDoc);

public slots:
    void onLists(QNetworkReply *reply); //slot for list download finished
    void onAssets(QNetworkReply *reply);
//    void onReadingFromServer();
signals:
    void onReady();
    void downloadedFileCountChanged(float percent, QString filepath);
    void loadingList();
    void error(QString msg);
    void versionNotify(QString version);

};


#endif // NETWORKMANAGER_H
