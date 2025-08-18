import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RowLayout {
    Layout.fillHeight: true
    Layout.preferredHeight: 0
    Layout.alignment: Qt.AlignHCenter
    spacing: 10

    Button {
        text: "Optimal Ball Collection\n(8 balls max)"
        onClicked: {
            var optimalRoute = pathfindingEngine.findOptimalBallCollectionRoute("start_a", "release", 8)
            if (optimalRoute.length > 0) {
                globalOptimalPath = optimalRoute
                fullMap.refresh()

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
            keyboardHandler.forceActiveFocus()
        }
    }

    Button {
        text: "High Value Route\n(Priority targets)"
        onClicked: {
            var highValueNodes = [
                "b16", "b17",
                "comm_tow"
            ]

            var optimalRoute = pathfindingEngine.findOptimalCollectionRoute("start_a", highValueNodes)
            if (optimalRoute.length > 0) {
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
            keyboardHandler.forceActiveFocus()
        }
    }

    Button {
        text: "Start B Route\n(Alternative start)"
        onClicked: {
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
            keyboardHandler.forceActiveFocus()
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
            keyboardHandler.forceActiveFocus()
        }
    }
}
