import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 2.1
import MyApp 1.0
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0


Window {
    visible: true
    width: 773
    height: 483
    title: qsTr("Omochim launcher")

    NetworkManager {
        id: test
    }

    MainPage {
        id:mainpage
    }

}
