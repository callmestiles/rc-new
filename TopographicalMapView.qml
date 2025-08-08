import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    width: 800
    height: 600

    property real scaleX: width / 500
    property real scaleY: height / 420
    property point robotPosition: Qt.point(50,50)
    property var optimalPath: []
    property bool showConnections: true
    property bool showOptimalPath: false

    property string hoveredNodeData: ""
    property point tooltipPosition: Qt.point(0, 0)
    property bool showTooltip: false

    property alias nodeModel: nodeModel

    ListModel {
        id: nodeModel

        //Start areas
        ListElement {x: 62; y: 62; type: "start_a"; elevation: 0; elementId:"start_a"}
        ListElement {x: 438; y: 338; type: "start_b"; elevation: 0; elementId:"start_b"}

        //Release areas
        ListElement {x: 398; y: 102; type: "release"; elevation: 0; elementId:"release"}

        //Keystone areas (Critical terrain features)
        //Hill 1 (Star Balls Area)
        ListElement {x: 135; y: 70; type: "keystone"; elevation: 0; elementId:"k1"}
        ListElement {x: 118; y: 250; type: "keystone"; elevation: 45; elementId:"k2"}
        ListElement {x: 120; y: 200; type: "keystone"; elevation: 36; elementId:"k3"}
        ListElement {x: 124; y: 170; type: "keystone"; elevation: 27; elementId:"k4"}
        ListElement {x: 128; y: 150; type: "keystone"; elevation: 18; elementId:"k5"}

        //Hill 2 (Communication Tower Hill)
        ListElement {x: 260; y: 220; type: "keystone"; elevation: 0; elementId:"k6"}
        ListElement {x: 340; y: 250; type: "keystone"; elevation: 0; elementId:"k7"}
        ListElement {x: 260; y: 280; type: "keystone"; elevation: 36; elementId:"k8"}
        ListElement {x: 258; y: 270; type: "keystone"; elevation: 27; elementId:"k9"}
        ListElement {x: 265; y: 220; type: "keystone"; elevation: 18; elementId:"k10"}

        //Strategic Passages
        ListElement {x: 280; y: 180; type: "keystone"; elevation: 0; elementId:"k11"}
        ListElement {x: 185; y: 205; type: "keystone"; elevation: 0; elementId:"k12"}
        ListElement {x: 180; y: 215; type: "keystone"; elevation: 9; elementId:"k13"}
        ListElement {x: 90; y: 138; type: "keystone"; elevation: 9; elementId:"k14"}

        //Ball Positions
        //Green Balls
        ListElement {x: 132; y: 26.5; type: "green_ball"; elevation: 0; elementId:"b1"; points: 5}
        ListElement {x: 218; y: 92; type: "green_ball"; elevation: 0; elementId:"b2"; points: 5}
        ListElement {x: 307; y: 152.5; type: "green_ball"; elevation: 0; elementId:"b3"; points: 5}
        ListElement {x: 374; y: 213; type: "green_ball"; elevation: 0; elementId:"b4"; points: 5}
        ListElement {x: 473.5; y: 257.5; type: "green_ball"; elevation: 0; elementId:"b5"; points: 5}

        //Black-stripped Balls
        ListElement {x: 71; y: 138; type: "black_striped_ball"; elevation: 27; elementId:"b6"; points: 10}
        ListElement {x: 38; y: 215; type: "black_striped_ball"; elevation: 18; elementId:"b7"; points: 10}
        ListElement {x: 48; y: 283; type: "black_striped_ball"; elevation: 9; elementId:"b8"; points: 10}
        ListElement {x: 71; y: 303; type: "black_striped_ball"; elevation: 9; elementId:"b9"; points: 10}
        ListElement {x: 62; y: 330; type: "black_striped_ball"; elevation: 9; elementId:"b10"; points: 10}
        ListElement {x: 70; y: 355; type: "black_striped_ball"; elevation: 9; elementId:"b11"; points: 10}
        ListElement {x: 80; y: 365; type: "black_striped_ball"; elevation: 9; elementId:"b12"; points: 10}
        ListElement {x: 90; y: 375; type: "black_striped_ball"; elevation: 9; elementId:"b13"; points: 10}
        ListElement {x: 256; y: 276; type: "black_striped_ball"; elevation: 27; elementId:"b14"; points: 10}
        ListElement {x: 252; y: 355; type: "black_striped_ball"; elevation: 27; elementId:"b15"; points: 10}

        //Star Balls
        ListElement {x: 71; y: 205; type: "star_ball"; elevation: 45; elementId:"b16"; points: 40}
        ListElement {x: 114; y: 230; type: "star_ball"; elevation: 45; elementId:"b17"; points: 40}

        //Communication Tower
        ListElement {x: 204; y: 327; type: "comm_tow"; elevation: 36; elementId:"comm_tow"; points: 60}

    }

    property var nodeConnections: ({})

    Component.onCompleted: {
        calculateNodeConnections()
    }

    // function getNodeByElementId(elementId) {
    //     for (let i = 0; i < nodeModel.count; i++) {
    //         let node = nodeModel.get(i)
    //         if (node.elementId === elementId) {
    //             return {
    //                 elementId: node.elementId,
    //                 x: node.x,
    //                 y: node.y,
    //                 elevation: node.elevation,
    //                 type: node.type,
    //                 points: node.points || 0
    //             }
    //         }
    //     }
    //     return null
    // }

    function calculateNodeConnections() {
        let connections = {}

        for (let i = 0; i < nodeModel.count; i++) {
            let node = nodeModel.get(i)
            let nodeConnections = []

            // Calculate distances to all other nodes
            let distances = []
            for (let j = 0; j < nodeModel.count; j++) {
                if (i === j) continue

                let otherNode = nodeModel.get(j)
                let distance = Math.sqrt(
                    Math.pow(node.x - otherNode.x, 2) +
                    Math.pow(node.y - otherNode.y, 2)
                )

                distances.push({
                    index: j,
                    distance: distance,
                    node: otherNode
                })
            }

            // Sort by distance and take closest 6
            distances.sort(function(a, b) { return a.distance - b.distance })

            for (let k = 0; k < Math.min(6, distances.length); k++) {
                nodeConnections.push({
                    targetIndex: distances[k].index,
                    distance: distances[k].distance,
                    cost: calculatePathCost(node, distances[k].node, distances[k].distance)
                })
            }

            connections[node.elementId] = nodeConnections
        }

        nodeConnections = connections
        // console.log(JSON.stringify(nodeConnections, null, 2))
    }

    function calculatePathCost(node1, node2, distance) {
        // Calculate cost based on distance and height difference
        let heightDiff = Math.abs(node1.elevation - node2.elevation)
        let terrainMultiplier = 1 + (heightDiff / 50) // Adjust as needed
        return distance * terrainMultiplier
    }

    function checkNodeHover(mouseX, mouseY) {
        for (let i = 0; i < nodeModel.count; i++) {
            let node = nodeModel.get(i)
            let nodeX = node.x * scaleX
            let nodeY = node.y * scaleY
            let radius = node.type === "keystone" ? 7 : node.type === "comm_tow" || node.type === "star_ball" ? 8 : 6

            let distance = Math.sqrt(Math.pow(mouseX - nodeX, 2) + Math.pow(mouseY - nodeY, 2))

            if (distance <= radius + 2) { // Add small tolerance
                // Show elevation for keystone, start, and release nodes
                if (node.type === "keystone" || node.type === "start_a" || node.type === "start_b" || node.type === "release") {
                    hoveredNodeData = "Elevation: " + node.elevation + "cm"
                }
                // Show points for ball nodes and communication tower
                else if (node.type === "green_ball" || node.type === "black_striped_ball" || node.type === "star_ball" || node.type === "comm_tow") {
                    hoveredNodeData = "Points: " + node.points
                }

                tooltipPosition = Qt.point(mouseX + 10, mouseY - 10)
                showTooltip = true
                return
            }
        }

        showTooltip = false
    }

    function getNodeByElementId(elementId) {
        for (let i = 0; i < nodeModel.count; i++) {
            let node = nodeModel.get(i)
            if (node.elementId === elementId) {
                return node
            }
        }
        return null
    }

    function refresh() {
       terrainMap.requestPaint()
   }

    Canvas {
        id: terrainMap
        anchors.fill: parent

        onPaint: {
            let ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            if(showConnections){
                drawNodeConnections(ctx)
            }

            if(showOptimalPath && optimalPath.length > 0){
                drawOptimalPath(ctx)
            }

            drawNodes(ctx)

            // drawRobot(ctx)
        }

        function drawNodes(ctx){
            for(let i=0; i < nodeModel.count; i++){
                let node = nodeModel.get(i)
                let x = node.x * scaleX
                let y = node.y * scaleY

                ctx.save()

                // Check if this node is in the optimal path
                let isInPath = false
                for (let j = 0; j < optimalPath.length; j++) {
                    if (optimalPath[j].elementId === node.elementId) {
                        isInPath = true
                        break
                    }
                }

                switch(node.type){
                    case "keystone":
                        ctx.fillStyle = isInPath ? "#FF6B35" : "#7B3F00"
                        ctx.strokeStyle = isInPath ? "#FF0000" : "#CDC1FF"
                        ctx.lineWidth = isInPath ? 3 : 1
                        break
                    case "start_a":
                        ctx.fillStyle = isInPath ? "#0066FF" : "blue"
                        ctx.strokeStyle = isInPath ? "#FFFFFF" : "black"
                        ctx.lineWidth = isInPath ? 3 : 1
                        break
                    case "start_b":
                        ctx.fillStyle = isInPath ? "#FF3366" : "red"
                        ctx.strokeStyle = isInPath ? "#FFFFFF" : "black"
                        ctx.lineWidth = isInPath ? 3 : 1
                        break
                    case "release":
                        ctx.fillStyle = isInPath ? "#66D9EF" : "#89A8B2"
                        ctx.strokeStyle = isInPath ? "#FFFFFF" : "#C1BAA1"
                        ctx.lineWidth = isInPath ? 3 : 2
                        break
                    case "green_ball":
                        ctx.fillStyle = isInPath ? "#66FF66" : "#41ab5d"
                        ctx.strokeStyle = isInPath ? "#FFFFFF" : "#238b45"
                        ctx.lineWidth = isInPath ? 3 : 1
                        break
                    case "black_striped_ball":
                        ctx.fillStyle = isInPath ? "#666666" : "black"
                        ctx.strokeStyle = isInPath ? "#FFFFFF" : "#74c476"
                        ctx.lineWidth = isInPath ? 3 : 1
                        break
                    case "star_ball":
                        ctx.fillStyle = isInPath ? "#FFD700" : "gold"
                        ctx.strokeStyle = isInPath ? "#FFFFFF" : "black"
                        ctx.lineWidth = isInPath ? 3 : 1
                        break
                    case "comm_tow":
                        ctx.fillStyle = isInPath ? "#FFFF66" : "yellow"
                        ctx.strokeStyle = isInPath ? "#FFFFFF" : "black"
                        ctx.lineWidth = isInPath ? 3 : 1
                        break
                }

                ctx.beginPath()
                ctx.arc(x,y, node.type === "keystone" ? 7 : node.type === "comm_tow" || node.type === "star_ball" ? 8 : 6, 0, 2 * Math.PI)
                ctx.fill()
                ctx.stroke()

                ctx.restore()
            }
        }

        function drawNodeConnections(ctx) {
           ctx.strokeStyle = "black"
           ctx.lineWidth = 1
           ctx.globalAlpha = 0.3

           for (let nodeId in nodeConnections) {
               let sourceNode = null
               let sourceIndex = -1

               // Find source node
               for (let i = 0; i < nodeModel.count; i++) {
                   if (nodeModel.get(i).elementId === nodeId) {
                       sourceNode = nodeModel.get(i)
                       sourceIndex = i
                       break
                   }
               }

               if (!sourceNode) continue

               let connections = nodeConnections[nodeId]
               for (let j = 0; j < connections.length; j++) {
                   let targetNode = nodeModel.get(connections[j].targetIndex)

                   ctx.beginPath()
                   ctx.moveTo(sourceNode.x * scaleX, sourceNode.y * scaleY)
                   ctx.lineTo(targetNode.x * scaleX, targetNode.y * scaleY)
                   ctx.stroke()
               }
           }

           ctx.globalAlpha = 1.0
        }

        function drawOptimalPath(ctx) {
            if (optimalPath.length < 2) return

            ctx.strokeStyle = "#FF0000"
            ctx.lineWidth = 4
            ctx.globalAlpha = 0.8

            // Draw path segments
            for (let k = 0; k < optimalPath.length - 1; k++) {
                let currentNode = optimalPath[k]
                let nextNode = optimalPath[k + 1]

                let x1 = currentNode.x * scaleX
                let y1 = currentNode.y * scaleY
                let x2 = nextNode.x * scaleX
                let y2 = nextNode.y * scaleY

                ctx.beginPath()
                ctx.moveTo(x1, y1)
                ctx.lineTo(x2, y2)
                ctx.stroke()
            }

            // Draw direction arrows
            ctx.fillStyle = "black"
            ctx.globalAlpha = 1.0

            for (let l = 0; l < optimalPath.length - 1; l++) {
                let currentNode = optimalPath[l]
                let nextNode = optimalPath[l + 1]

                let x1 = currentNode.x * scaleX
                let y1 = currentNode.y * scaleY
                let x2 = nextNode.x * scaleX
                let y2 = nextNode.y * scaleY

                // Calculate arrow position (midpoint)
                let midX = (x1 + x2) / 2
                let midY = (y1 + y2) / 2

                // Calculate angle
                let angle = Math.atan2(y2 - y1, x2 - x1)

                // Draw arrow
                ctx.save()
                ctx.translate(midX, midY)
                ctx.rotate(angle)

                ctx.beginPath()
                ctx.moveTo(8, 0)
                ctx.lineTo(-4, -3)
                ctx.lineTo(-4, 3)
                ctx.closePath()
                ctx.fill()

                ctx.restore()
            }

            // Draw path sequence numbers
            ctx.fillStyle = "#FFFFFF"
            ctx.font = "12px Arial"
            ctx.textAlign = "center"
            ctx.textBaseline = "middle"

            for (let i = 0; i < optimalPath.length; i++) {
                let node = optimalPath[i]
                let x = node.x * scaleX
                let y = node.y * scaleY

                // Draw background circle for number
                ctx.fillStyle = "black"
                ctx.beginPath()
                ctx.arc(x + 10, y - 10, 8, 0, 2 * Math.PI)
                ctx.fill()

                // Draw number
                ctx.fillStyle = "#FFFFFF"
                ctx.fillText((i + 1).toString(), x + 10, y - 10)
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            onPositionChanged: {
                checkNodeHover(mouseX, mouseY)
            }

            onExited: {
                showTooltip = false
            }
        }
    }

    // Tooltip
    Rectangle {
        id: tooltip
        visible: showTooltip
        x: Math.min(tooltipPosition.x, root.width - width)
        y: Math.max(0, Math.min(tooltipPosition.y, root.height - height))
        width: tooltipText.width + 16
        height: tooltipText.height + 8
        color: "#FFFFCC"
        border.color: "#888888"
        border.width: 1
        radius: 4
        z: 1000

        Text {
            id: tooltipText
            text: hoveredNodeData
            anchors.centerIn: parent
            font.pixelSize: 11
            color: "#333333"
        }
    }
}
