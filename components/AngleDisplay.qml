import QtQuick 2.15

Rectangle {
    property real angle: 0
    // property color color: "gray"

    width: 60; height: 25
    radius: 4
    color: parent.color

    Text {
        anchors.centerIn: parent
        text: angle + "°"
        color: "white"
        font.bold: true
        font.pointSize: 10
    }
}
