import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import PathfindingEngine

import "./src/components"
import "./src/pages"

Window {
    id: mainWindow
    visible: true
    title: qsTr("Transformers GUI")
    visibility: "Maximized"

    property var globalOptimalPath: []

    PathfindingEngine {
        id: pathfindingEngine
    }

    StackView{
        id: stackView
        anchors.fill: parent
        initialItem: control

        Component {
            id: control
            ControlPage{}
        }

        Component {
            id: map
            MapPage{}
        }
    }
}
