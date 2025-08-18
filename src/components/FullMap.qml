import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    color: "#F0E4D3"
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredHeight: 0

    TopographicalMapView {
        id: fullMap
        anchors.fill: parent
        anchors.margins: 2

        showConnections: true
        showOptimalPath: true
        optimalPath: globalOptimalPath

        Component.onCompleted: {
            initializePathfindingEngineLocal()
        }

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
