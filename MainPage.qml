import QtQuick 2.6
import QtQuick.Controls 2.1
import MyApp 1.0
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0

Rectangle {
    id: container
    width: 773
    height: 483
    color: "#00000000"
    Settings {
        id: localStorage
        property bool agreedToTerms:  false
    }
    NetworkManager {
        id: networkmanager
    }


    Image {
        id: image
        width: container.width
        height: container.height
        z: -1
        source: "bg.png"
    }

    Rectangle {
        id: percent_text_container
        x: 44
        y: 368
        width: 608
        height: 62
        anchors.horizontalCenter:parent.horizontalCenter
        color: "#00000000"
        anchors.horizontalCenterOffset: 6
        Text {
            id: percent_text
            text: qsTr("")
            anchors.top: parent.top
            anchors.topMargin: 5
            anchors.horizontalCenter: parent.horizontalCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 12
            Connections {
                target: networkmanager // Make sure the object name matches
                onDownloadedFileCountChanged: {
                    // Update the ProgressBar value when the count changes
                    percent_text.text = Math.round(percent*100) + qsTr("% completed")
                }
            }
            Connections{
                target: networkmanager
                onLoadingList:{
                    percent_text.text = "downloading updated list";
                }
            }
        }
        ProgressBar {
            id: progressBar
            //            anchors.centerIn: parent
            width: parent.width
            height: 20
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            value:0
            background: Rectangle {
                implicitWidth: 200
                implicitHeight: 6
                color: "#e6e6e6"
                radius: 3
                border.color: "black"
            }

            contentItem: Item {
                implicitWidth: 200
                implicitHeight: 4

                Rectangle {
                    width: progressBar.value*progressBar.width
                    height: parent.height
                    radius: 2
                    color: "#E06666"
                    border.color: "black"
                }
            }
            Connections {
                target: networkmanager // Make sure the object name matches
                onDownloadedFileCountChanged: {
                    // Update the ProgressBar value when the count changes
                    progressBar.value = percent;
                }
            }
        }
    }

    RowLayout {
        id: row
        x: 318
        y: 436
        width: 447
        height: 39
        spacing: 0
        Text {
            id: installer_v
            text: qsTr("LauncherVersion: ")
            font.bold: true
            font.pixelSize: 16
        }

        Text {
            id: app_v
            text: qsTr("AppVersion: ")
            font.bold: true
            font.pixelSize: 16
        }
        Connections{
            target: networkmanager
            onVersionNotify:{
                app_v.text+=version;
            }
        }
    }
    Popup{
        id:error
        modal:true
        width:parent.width*0.9
        height: parent.height*0.5
        x:(parent.width-width)/2
        y:(parent.height-height)/2
        visible: false
        closePolicy: Popup.CloseOnEscape | Popup.NoAutoClose
        padding:20
        background: Rectangle {
            color: "#666666" // Set the background color of the Popup to transparent
            border.color: "black" // Set the border color to transparent
        }
        contentItem: Column{
            Text{
                id:error_msg
                text:""
                font.pixelSize: 16
                color: "white"
                Connections{
                    target: networkmanager
                    onError:{
                        error_msg.text=msg;
                        error.visible = true;
                    }
                }
            }
        }
    }

    Popup {
        id: terms
        modal: true
        width: parent.width*0.9
        height: parent.height*0.9
        x:(parent.width-width)/2
        y:(parent.height-height)/2
        visible: false
        closePolicy: Popup.CloseOnEscape | Popup.NoAutoClose
        padding:40
        leftPadding: 70
        rightPadding: 70
        contentItem: ColumnLayout{
            Column{
                spacing: 10
                Text{
                    text:"1.利用規約に同意いただけた場合のみ本ゲームのプレイが可能です。"
                    font.pixelSize: 20
                    color: "white"
                }
                Text{
                    text:"2.未成年者は、親権者等の法定代理人の同意が必要です。"
                    color: "white"
                    font.pixelSize: 20
                }
            }
            Text{
                text:"利用規約を確認すると同意するボタンが有効になります。"
                color: "white"
                font.pixelSize: 20
            }
            Column{
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 20
                Button{
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "利用規約を確認する"
                    font.pixelSize: 16
                    onClicked: {
                        var url = "https://app.omochim.com/players/termsOfService";
                        Qt.openUrlExternally(url);

                    }
                }
                Row{
                    spacing: 40
                    Button{
                        text:"終了する"
                        font.pixelSize: 16
                        onClicked: {
                            Qt.quit();
                        }
                    }
                    Button{
                        text:"同意する"
                        font.pixelSize: 16
                        onClicked: {
                            localStorage.agreedToTerms=true
                            terms.visible=false;
                            networkmanager.initialize();
                        }
                    }
                }
            }
        }

        background: Rectangle {
            color: "#666666" // Set the background color of the Popup to transparent
            border.color: "black" // Set the border color to transparent
        }
    }
    Component.onCompleted:{
        if (!localStorage.agreedToTerms) {
            // Show the terms and services dialog
            terms.visible=true;
        }
        else
        {
            networkmanager.initialize();
        }
    }


}

