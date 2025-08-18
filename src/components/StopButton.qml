import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: stopButton
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.leftMargin: 115
    width: 100
    height: 100
    radius: width / 2
    color: "red"

    border.color: "black"
    border.width: 2

    Text {
        anchors.centerIn: parent
        text: "STOP"
        color: "white"
        font.bold: true
        font.pixelSize: 24
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            carController.stopCar()
        }
    }
}
