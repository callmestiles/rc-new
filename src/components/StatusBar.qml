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
        label: carController.isConnected ? "Car Connected" : "Car Disconnected"
        labelColor: carController.isConnected ? "green" : "red"
        circleColor: carController.isConnected ? "green" : "red"
    }
}
