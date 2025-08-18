import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "../components"

Page {
    title: "Map Details"

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            spacing: 20

            Text {
                text: "Map Details"
                font.pointSize: 20
                Layout.leftMargin: 10
                Layout.topMargin: 5
            }

            Button {
                text: "Back"
                onClicked: {
                    stackView.replace(control)
                    keyboardHandler.forceActiveFocus()
                }
                Layout.alignment: Qt.AlignVCenter
            }
        }

        FullMap{}

        MapControls{}

        PathStatus{}
    }
}
