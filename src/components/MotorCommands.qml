import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.rightMargin: 10
    width: 160
    height: 110
    color: "white"
    radius: 5
    border.color: "grey"

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 5

        Text {
            text: "Motor Commands"
            font.bold: true
            font.pointSize: 12
        }

        Text {
            text: "Left: " + carController.leftMotorSpeed
            font.pointSize: 10
        }

        Text {
            text: "Right: " + carController.rightMotorSpeed
            font.pointSize: 10
        }

        Text {
            text: "Status: " + (carController.leftMotorSpeed === 0 && carController.rightMotorSpeed === 0 ? "Stopped" : "Moving")
            font.pointSize: 10
            color: carController.leftMotorSpeed === 0 && carController.rightMotorSpeed === 0 ? "red" : "green"
        }
    }
}
