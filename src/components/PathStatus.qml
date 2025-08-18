import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

RowLayout {
    Layout.fillHeight: true
    Layout.preferredHeight: 0
    Layout.alignment: Qt.AlignHCenter
    spacing: 20

    Text {
        text: "Path Length: " + globalOptimalPath.length + " nodes"
        font.pixelSize: 20
        color: "#666666"

    }

    Text {
        id: pathStatusText
        text: globalOptimalPath.length > 0 ? "Path Status: Active" : "Path Status: None"
        font.pixelSize: 20
        color: globalOptimalPath.length > 0 ? "#2ecc71" : "#e74c3c"
    }
}
