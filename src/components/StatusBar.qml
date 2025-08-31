import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    id: statusBar
    spacing: 20

    Text {
        id: title
        text: "Control Center"
        font.pointSize: 20
    }

    //StatusIndicator goes here
    StatusIndicator {
        label: networkManager.isConnected ? "Car Connected" : "Car Disconnected"
        labelColor: networkManager.isConnected ? "green" : "red"
        circleColor: networkManager.isConnected ? "green" : "red"
    }
}
