import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    property alias label: label.text
    property alias circleColor: statusCircle.color
    property alias labelColor: label.color

    Rectangle {
        id: statusCircle
        width: 20
        height: 20
        radius: 10
        color: "red"
    }

    Text {
        id: label
        text: "Disconnected"
        color: "red"
        font.pointSize: 12
    }
}
