import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    Layout.leftMargin: 20
    width: 150
    height: 60
    color: "lightblue"
    opacity: 0.8
    radius: 5
    border.color: "darkblue"
    border.width: 1

    Column {
        anchors.centerIn: parent
        spacing: 2

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "WASD Controls"
            font.bold: true
            font.pointSize: 10
            color: "darkblue"
        }

        Grid {
            anchors.horizontalCenter: parent.horizontalCenter
            columns: 3
            spacing: 2

            Rectangle { width: 20; height: 15; color: "transparent" }
            Rectangle {
                width: 20; height: 15;
                color: keyboardHandler.wPressed ? "green" : "lightgray"
                radius: 2
                Text {
                    anchors.centerIn: parent
                    text: "W"
                    font.pointSize: 8
                    color: keyboardHandler.wPressed ? "white" : "black"
                }
            }
            Rectangle { width: 20; height: 15; color: "transparent" }

            Rectangle {
                width: 20; height: 15;
                color: keyboardHandler.aPressed ? "green" : "lightgray"
                radius: 2
                Text {
                    anchors.centerIn: parent
                    text: "A"
                    font.pointSize: 8
                    color: keyboardHandler.aPressed ? "white" : "black"
                }
            }
            Rectangle {
                width: 20; height: 15;
                color: keyboardHandler.sPressed ? "green" : "lightgray"
                radius: 2
                Text {
                    anchors.centerIn: parent
                    text: "S"
                    font.pointSize: 8
                    color: keyboardHandler.sPressed ? "white" : "black"
                }
            }
            Rectangle {
                width: 20; height: 15;
                color: keyboardHandler.dPressed ? "green" : "lightgray"
                radius: 2
                Text {
                    anchors.centerIn: parent
                    text: "D"
                    font.pointSize: 8
                    color: keyboardHandler.dPressed ? "white" : "black"
                }
            }
        }
    }
}
