import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import PathfindingEngine

Window {
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

            Page {
                objectName: "controlPage" // Add objectName for identification

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    RowLayout {
                        spacing: 20

                        Text {
                            id: title
                            text: "Control Center"
                            font.pointSize: 20
                        }
                    }

                    Rectangle { //this will be the actual camera feed from micro-controller
                        color: "lightgrey"
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        ColumnLayout{
                            anchors.fill: parent

                            RowLayout {
                                anchors.fill: parent
                                //MINI MAP
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

                                    // Mini terrain map
                                    TopographicalMapView {
                                        id: miniMap
                                        anchors.fill: parent
                                        anchors.margins: 5
                                        showConnections: false // Hide connections in mini-map for cleaner view
                                        showOptimalPath: true
                                        optimalPath: globalOptimalPath // Bind to global path

                                        // Watch for changes in globalOptimalPath and refresh
                                        Connections {
                                            target: application
                                            function onGlobalOptimalPathChanged() {
                                                miniMap.refresh()
                                            }
                                        }
                                    }

                                    // Mini-map title
                                    Text {
                                        anchors.top: parent.top
                                        anchors.left: parent.left
                                        anchors.margins: 5
                                        text: "Mini Map"
                                        font.bold: true
                                        font.pixelSize: 12
                                        color: "#333333"
                                    }

                                    // Click to expand
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            stackView.push(map)
                                        }
                                        cursorShape: Qt.PointingHandCursor
                                    }
                                }

                                //CAMERA VIEW
                                Rectangle {
                                    color: "darkgrey"
                                    width: 600
                                    height: 300
                                    anchors.top: parent.top
                                    anchors.left: parent.left
                                    anchors.margins: 10
                                    radius: 5
                                    border.color: "grey"
                                    border.width: 1

                                    Text{
                                        text: "Camera View"
                                        font.pointSize: 18
                                        anchors.centerIn: parent
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.preferredHeight: 0
                                color: "transparent"

                                // Vertical slider at bottom left
                                Slider {
                                    id: verticalSlider
                                    orientation: Qt.Vertical
                                    anchors.bottom: parent.bottom
                                    anchors.left: parent.left
                                    anchors.margins: 20
                                    height: 250
                                    width: 40
                                    from: -255
                                    to: 255
                                    value: 0

                                    background: Rectangle {
                                        anchors.fill: parent
                                        implicitWidth: 4
                                        color: "darkgrey"
                                        radius: 2
                                    }

                                    // Custom handle for better visibility
                                    handle: Rectangle {
                                        x: verticalSlider.leftPadding + (verticalSlider.horizontal ? verticalSlider.visualPosition * (verticalSlider.availableWidth - width) : (verticalSlider.availableWidth - width) / 2)
                                        y: verticalSlider.topPadding + (verticalSlider.horizontal ? (verticalSlider.availableHeight - height) / 2 : verticalSlider.visualPosition * (verticalSlider.availableHeight - height))
                                        implicitWidth: 50
                                        implicitHeight: 50
                                        radius: 25
                                        color: "#3E5F44"
                                        border.color: "#bdbebf"
                                    }

                                    // Optional: Add value display
                                    Text {
                                        anchors.left: parent.right
                                        anchors.leftMargin: 10
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: Math.round(verticalSlider.value)
                                        color: "black"
                                        font.pointSize: 18
                                    }
                                }

                                // Horizontal slider at bottom right
                                Slider {
                                    id: horizontalSlider
                                    orientation: Qt.Horizontal
                                    anchors.bottom: parent.bottom
                                    anchors.right: parent.right
                                    anchors.margins: 20
                                    width: 200
                                    height: 40
                                    from: 0
                                    to: 100
                                    value: 50

                                    // Custom handle for better visibility
                                    handle: Rectangle {
                                        x: horizontalSlider.leftPadding + (horizontalSlider.horizontal ? horizontalSlider.visualPosition * (horizontalSlider.availableWidth - width) : (horizontalSlider.availableWidth - width) / 2)
                                        y: horizontalSlider.topPadding + (horizontalSlider.horizontal ? (horizontalSlider.availableHeight - height) / 2 : horizontalSlider.visualPosition * (horizontalSlider.availableHeight - height))
                                        implicitWidth: 26
                                        implicitHeight: 26
                                        radius: 13
                                        color: "black"
                                        border.color: "#bdbebf"
                                    }

                                    // Optional: Add value display
                                    Text {
                                        anchors.top: parent.bottom
                                        anchors.topMargin: 5
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: Math.round(horizontalSlider.value)
                                        color: "black"
                                        font.pointSize: 18
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        Component {
            id: map

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
                            }
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }

                    Rectangle { //Full-size map view
                        color: "#F0E4D3"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredHeight: 0

                        // Full terrain map
                        TopographicalMapView {
                            id: fullMap
                            anchors.fill: parent
                            anchors.margins: 2

                            showConnections: true
                            showOptimalPath: true
                            optimalPath: globalOptimalPath // Bind to global path

                            // Initialize pathfinding engine with map data
                            Component.onCompleted: {
                                initializePathfindingEngineLocal()
                            }

                            // Watch for changes in globalOptimalPath and refresh
                            Connections {
                                target: application
                                function onGlobalOptimalPathChanged() {
                                    fullMap.refresh()
                                }
                            }

                            function initializePathfindingEngineLocal() {
                                var nodeArray = []
                                for (var i = 0; i < fullMap.nodeModel.count; i++) {
                                    var node = fullMap.nodeModel.get(i)
                                    nodeArray.push({
                                        elementId: node.elementId,
                                        x: node.x,
                                        y: node.y,
                                        elevation: node.elevation,
                                        type: node.type,
                                        points: node.points || 0
                                    })
                                }

                                pathfindingEngine.setNodes(nodeArray)

                                var connectionMap = {}
                                var connections = fullMap.nodeConnections
                                for (var nodeId in connections) {
                                    var nodeConnections = connections[nodeId]
                                    var connectionArray = []

                                    for (var j = 0; j < nodeConnections.length; j++) {
                                        var conn = nodeConnections[j]
                                        if (conn.targetIndex < fullMap.nodeModel.count) {
                                            var targetNode = fullMap.nodeModel.get(conn.targetIndex)
                                            connectionArray.push({
                                                targetId: targetNode.elementId,
                                                cost: conn.cost,
                                                distance: conn.distance,
                                                targetIndex: conn.targetIndex
                                            })
                                        }
                                    }
                                    connectionMap[nodeId] = connectionArray
                                }

                                pathfindingEngine.setConnections(connectionMap)
                                console.log("Pathfinding engine initialized")
                            }

                        }
                    }

                    // Replace the existing control buttons RowLayout in Main.qml with this:

                    RowLayout {
                        Layout.fillHeight: true
                        Layout.preferredHeight: 0
                        Layout.alignment: Qt.AlignHCenter
                        spacing: 10

                        Button {
                            text: "Optimal Ball Collection\n(8 balls max)"
                            onClicked: {
                                // Find optimal route collecting up to 8 balls and returning to release
                                var optimalRoute = pathfindingEngine.findOptimalBallCollectionRoute("start_a", "release", 8)
                                if (optimalRoute.length > 0) {
                                    globalOptimalPath = optimalRoute
                                    fullMap.refresh()

                                    // Calculate total points
                                    var totalPoints = 0
                                    for (var i = 0; i < optimalRoute.length; i++) {
                                        if (optimalRoute[i].points) {
                                            totalPoints += optimalRoute[i].points
                                        }
                                    }

                                    console.log("Optimal collection route found with", optimalRoute.length, "nodes")
                                    console.log("Total points:", totalPoints)
                                    pathStatusText.text = "Route: " + optimalRoute.length + " nodes, " + totalPoints + " points"
                                } else {
                                    console.log("No optimal collection route found")
                                    pathStatusText.text = "No route found"
                                }
                            }
                        }

                        Button {
                            text: "High Value Route\n(Priority targets)"
                            onClicked: {
                                // Find route focusing on high-value targets only
                                var highValueNodes = [
                                    "b16", "b17", // Star balls (40 points each)
                                    "comm_tow"    // Communication tower (60 points)
                                ]

                                var optimalRoute = pathfindingEngine.findOptimalCollectionRoute("start_a", highValueNodes)
                                if (optimalRoute.length > 0) {
                                    // Add release area to the end
                                    var releaseNode = fullMap.getNodeByElementId("release")
                                    if (releaseNode) {
                                        optimalRoute.push(releaseNode)
                                    }

                                    globalOptimalPath = optimalRoute
                                    fullMap.refresh()

                                    var totalPoints = 0
                                    for (var i = 0; i < optimalRoute.length; i++) {
                                        if (optimalRoute[i].points) {
                                            totalPoints += optimalRoute[i].points
                                        }
                                    }

                                    console.log("High value route found with", optimalRoute.length, "nodes")
                                    console.log("Total points:", totalPoints)
                                    pathStatusText.text = "Route: " + optimalRoute.length + " nodes, " + totalPoints + " points"
                                } else {
                                    console.log("No high value route found")
                                    pathStatusText.text = "No route found"
                                }
                            }
                        }

                        Button {
                            text: "Start B Route\n(Alternative start)"
                            onClicked: {
                                // Find optimal route from start_b
                                var optimalRoute = pathfindingEngine.findOptimalBallCollectionRoute("start_b", "release", 8)
                                if (optimalRoute.length > 0) {
                                    globalOptimalPath = optimalRoute
                                    fullMap.refresh()

                                    var totalPoints = 0
                                    for (var i = 0; i < optimalRoute.length; i++) {
                                        if (optimalRoute[i].points) {
                                            totalPoints += optimalRoute[i].points
                                        }
                                    }

                                    console.log("Start B route found with", optimalRoute.length, "nodes")
                                    console.log("Total points:", totalPoints)
                                    pathStatusText.text = "Route: " + optimalRoute.length + " nodes, " + totalPoints + " points"
                                } else {
                                    console.log("No route found from start B")
                                    pathStatusText.text = "No route found"
                                }
                            }
                        }

                        Button {
                            text: "Clear Path"
                            onClicked: {
                                globalOptimalPath = []
                                pathfindingEngine.clearPath()
                                fullMap.refresh()
                                pathStatusText.text = "Path Status: None"
                                console.log("Path cleared")
                            }
                        }
                    }

                    // Update the status row to show more detailed information:
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
                }
            }
        }
    }


}
