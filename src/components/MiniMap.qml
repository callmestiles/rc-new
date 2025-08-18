import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    color: "#F0E4D3"
    width: 400
    height: 300
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.margins: 10
    radius: 5
    border.color: "grey"
    border.width: 1

    TopographicalMapView {
        id: miniMap
        anchors.fill: parent
        anchors.margins: 5
        showConnections: false
        showOptimalPath: true
        optimalPath: globalOptimalPath

        Connections {
            target: application
            function onGlobalOptimalPathChanged() {
                miniMap.refresh()
            }
        }
    }

    Text {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 5
        text: "Mini Map"
        font.bold: true
        font.pixelSize: 12
        color: "#333333"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            stackView.push(map)
        }
        cursorShape: Qt.PointingHandCursor
    }
}
