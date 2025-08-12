import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import PathfindingEngine
import CarController
import ArmController

Window {
    id: mainWindow
    visible: true
    title: qsTr("Transformers GUI")
    visibility: "Maximized"

    property var globalOptimalPath: []

    function updateSliderValues() {
        var currentPage = stackView.currentItem
        if (currentPage && currentPage.objectName === "controlPage") {
            var speedValue
            if (keyboardHandler.wPressed && !keyboardHandler.sPressed) {
                speedValue = 255
            } else if (keyboardHandler.sPressed && !keyboardHandler.wPressed) {
                speedValue = -255
            } else if (keyboardHandler.wPressed && keyboardHandler.sPressed) {
                speedValue = 0
            } else if (keyboardHandler.autoReturnEnabled) {
                speedValue = 0
            } else {
                speedValue = currentPage.getVerticalSliderValue()
            }

            currentPage.setVerticalSliderValue(speedValue)

            var turnValue
            if (keyboardHandler.aPressed && !keyboardHandler.dPressed) {
                turnValue = -50
            } else if (keyboardHandler.dPressed && !keyboardHandler.aPressed) {
                turnValue = 50
            } else if (keyboardHandler.aPressed && keyboardHandler.dPressed) {
                turnValue = 0
            } else if (keyboardHandler.autoTurnEnabled) {
                turnValue = 0
            } else {
                turnValue = currentPage.getHorizontalSliderValue()
            }

            currentPage.setHorizontalSliderValue(turnValue)
        }
    }

    PathfindingEngine {
        id: pathfindingEngine
    }

    Component.onCompleted: {
        forceActiveFocus()
    }

    KeyboardHandler {
        id: keyboardHandler
        onKeysChanged: updateSliderValues()
    }

    StackView{
        id: stackView
        anchors.fill: parent
        initialItem: control

        Component {
            id: control

            Page {
                objectName: "controlPage"

                function setVerticalSliderValue(value) {
                    verticalSlider.value = value
                }

                function setHorizontalSliderValue(value) {
                    horizontalSlider.value = value
                }

                function getVerticalSliderValue() {
                    return verticalSlider.value
                }

                function getHorizontalSliderValue() {
                    return horizontalSlider.value
                }

                CarController {
                    id: carController
                    serverUrl: "http://192.168.4.1/setSpeed"
                    speedDeadZone: 10
                    turnDeadZone: 5
                }

                ArmController {
                    id: armController
                    serverUrl: "http://192.168.4.1/setServo"
                }

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

                        Rectangle {
                            width: 20
                            height: 20
                            radius: 10
                            color: carController.isConnected ? "green" : "red"
                        }

                        Text {
                            text: carController.isConnected ? "Connected" : "Disconnected"
                            color: carController.isConnected ? "green" : "red"
                            font.pointSize: 12
                        }

                        Rectangle {
                            width: 20
                            height: 20
                            radius: 10
                            color: armController.isConnected ? "green" : "red"
                        }

                        Text {
                            text: armController.isConnected ? "Arm Connected" : "Arm Disconnected"
                            color: armController.isConnected ? "green" : "red"
                            font.pointSize: 12
                        }

                        RowLayout {
                            Layout.leftMargin: 100
                            spacing: 10

                            Text {
                                text: "Auto center:"
                                font.pointSize: 12
                                color: "#333333"
                            }

                            Switch {
                                checked: keyboardHandler.autoReturnEnabled
                                onCheckedChanged: {
                                    if (checked !== keyboardHandler.autoReturnEnabled) {
                                        keyboardHandler.autoReturnEnabled = checked
                                        console.log("Auto center", checked ? "enabled" : "disabled")
                                    }
                                    keyboardHandler.forceActiveFocus()
                                }
                            }
                        }

                        RowLayout {
                            Layout.leftMargin: 20
                            spacing: 10

                            Text {
                                text: "Auto turn:"
                                font.pointSize: 12
                                color: "#333333"
                            }

                            Switch {
                                checked: keyboardHandler.autoTurnEnabled
                                onCheckedChanged: {
                                    if (checked !== keyboardHandler.autoTurnEnabled) {
                                        keyboardHandler.autoTurnEnabled = checked
                                        console.log("Auto turn", checked ? "enabled" : "disabled")
                                    }
                                    keyboardHandler.forceActiveFocus()
                                }
                            }
                        }

                        Rectangle {
                            Layout.leftMargin: 20
                            width: 150
                            height: 60
                            color: "lightblue"
                            opacity: 0.8
                            radius: 5
                            border.color: "darkblue"
                            border.width: 1

                            Column {
                                anchors.centerIn: parent
                                spacing: 2

                                Text {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    text: "WASD Controls"
                                    font.bold: true
                                    font.pointSize: 10
                                    color: "darkblue"
                                }

                                Grid {
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    columns: 3
                                    spacing: 2

                                    Rectangle { width: 20; height: 15; color: "transparent" }
                                    Rectangle {
                                        width: 20; height: 15;
                                        color: keyboardHandler.wPressed ? "green" : "lightgray"
                                        radius: 2
                                        Text {
                                            anchors.centerIn: parent
                                            text: "W"
                                            font.pointSize: 8
                                            color: keyboardHandler.wPressed ? "white" : "black"
                                        }
                                    }
                                    Rectangle { width: 20; height: 15; color: "transparent" }

                                    Rectangle {
                                        width: 20; height: 15;
                                        color: keyboardHandler.aPressed ? "green" : "lightgray"
                                        radius: 2
                                        Text {
                                            anchors.centerIn: parent
                                            text: "A"
                                            font.pointSize: 8
                                            color: keyboardHandler.aPressed ? "white" : "black"
                                        }
                                    }
                                    Rectangle {
                                        width: 20; height: 15;
                                        color: keyboardHandler.sPressed ? "green" : "lightgray"
                                        radius: 2
                                        Text {
                                            anchors.centerIn: parent
                                            text: "S"
                                            font.pointSize: 8
                                            color: keyboardHandler.sPressed ? "white" : "black"
                                        }
                                    }
                                    Rectangle {
                                        width: 20; height: 15;
                                        color: keyboardHandler.dPressed ? "green" : "lightgray"
                                        radius: 2
                                        Text {
                                            anchors.centerIn: parent
                                            text: "D"
                                            font.pointSize: 8
                                            color: keyboardHandler.dPressed ? "white" : "black"
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        color: "lightgrey"
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        ColumnLayout{
                            anchors.fill: parent

                            Rectangle{
                                color: "transparent"
                                Layout.fillWidth: true
                                Layout.preferredHeight: 400

                                RowLayout {
                                    anchors.fill: parent

                                    // ARM CONTROL PANEL - CENTERED
                                   Rectangle {
                                       Layout.leftMargin: 180
                                       width: 500
                                       height: 380
                                       color: "#FAF9EE"
                                       radius: 10
                                       border.color: "black"
                                       border.width: 2

                                       ColumnLayout {
                                           anchors.fill: parent
                                           anchors.margins: 15
                                           spacing: 15

                                           Text {
                                               text: "Robotic Arm Control"
                                               font.bold: true
                                               font.pointSize: 16
                                               color: "#2C3E50"
                                               Layout.alignment: Qt.AlignHCenter
                                           }

                                           // Position Control Section
                                           Rectangle {
                                               Layout.fillWidth: true
                                               height: 180
                                               color: "white"
                                               radius: 8
                                               border.color: "#BDC3C7"
                                               border.width: 1

                                               ColumnLayout {
                                                   anchors.fill: parent
                                                   anchors.margins: 10
                                                   spacing: 10

                                                   Text {
                                                       text: "Position Control (Inverse Kinematics)"
                                                       font.bold: true
                                                       font.pointSize: 12
                                                       color: "#34495E"
                                                   }

                                                   GridLayout {
                                                       columns: 3
                                                       columnSpacing: 15
                                                       rowSpacing: 10

                                                       // X Control
                                                       Text { text: "X:"; font.pointSize: 10; color: "#7F8C8D" }
                                                       Slider {
                                                           id: xSlider
                                                           from: 5
                                                           to: 35
                                                           value: armController.targetX
                                                           Layout.preferredWidth: 120
                                                           onValueChanged: {
                                                               if (pressed) armController.targetX = value
                                                           }
                                                       }
                                                       Text {
                                                           text: armController.targetX.toFixed(1) + " cm"
                                                           font.pointSize: 10
                                                           color: "#2C3E50"
                                                           Layout.preferredWidth: 50
                                                       }

                                                       // Y Control
                                                       Text { text: "Y:"; font.pointSize: 10; color: "#7F8C8D" }
                                                       Slider {
                                                           id: ySlider
                                                           from: -20
                                                           to: 20
                                                           value: armController.targetY
                                                           Layout.preferredWidth: 120
                                                           onValueChanged: {
                                                               if (pressed) armController.targetY = value
                                                           }
                                                       }
                                                       Text {
                                                           text: armController.targetY.toFixed(1) + " cm"
                                                           font.pointSize: 10
                                                           color: "#2C3E50"
                                                           Layout.preferredWidth: 50
                                                       }

                                                       // Z Control
                                                       Text { text: "Z:"; font.pointSize: 10; color: "#7F8C8D" }
                                                       Slider {
                                                           id: zSlider
                                                           from: 0
                                                           to: 30
                                                           value: armController.targetZ
                                                           Layout.preferredWidth: 120
                                                           onValueChanged: {
                                                               if (pressed) armController.targetZ = value
                                                           }
                                                       }
                                                       Text {
                                                           text: armController.targetZ.toFixed(1) + " cm"
                                                           font.pointSize: 10
                                                           color: "#2C3E50"
                                                           Layout.preferredWidth: 50
                                                       }
                                                   }

                                                   RowLayout {
                                                       Layout.alignment: Qt.AlignHCenter
                                                       spacing: 10

                                                       Button {
                                                           text: "Home Position"
                                                           onClicked: {
                                                               armController.resetToHome()
                                                               keyboardHandler.forceActiveFocus()
                                                           }
                                                       }

                                                       Button {
                                                           text: "Apply Position"
                                                           onClicked: {
                                                               armController.moveToPosition(xSlider.value, ySlider.value, zSlider.value)
                                                               keyboardHandler.forceActiveFocus()
                                                           }
                                                           // enabled: armController.isConnected
                                                       }
                                                   }
                                               }
                                           }

                                           // Servo Angle Display
                                           Rectangle {
                                               Layout.fillWidth: true
                                               height: 120
                                               color: "white"
                                               radius: 8
                                               border.color: "#BDC3C7"
                                               border.width: 1

                                               ColumnLayout {
                                                   anchors.fill: parent
                                                   anchors.margins: 10
                                                   spacing: 8

                                                   Text {
                                                       text: "Servo Angles"
                                                       font.bold: true
                                                       font.pointSize: 12
                                                       color: "#34495E"
                                                   }

                                                   GridLayout {
                                                       columns: 4
                                                       columnSpacing: 20
                                                       rowSpacing: 5

                                                       Text { text: "Base"; font.bold: true; font.pointSize: 10; color: "#7F8C8D" }
                                                       Text { text: "Shoulder"; font.bold: true; font.pointSize: 10; color: "#7F8C8D" }
                                                       Text { text: "Elbow"; font.bold: true; font.pointSize: 10; color: "#7F8C8D" }
                                                       Text { text: "Wrist"; font.bold: true; font.pointSize: 10; color: "#7F8C8D" }

                                                       Rectangle {
                                                           width: 60; height: 25
                                                           color: "#3498DB"
                                                           radius: 4
                                                           Text {
                                                               anchors.centerIn: parent
                                                               text: armController.baseAngle + "°"
                                                               color: "white"
                                                               font.bold: true
                                                               font.pointSize: 10
                                                           }
                                                       }

                                                       Rectangle {
                                                           width: 60; height: 25
                                                           color: "#E74C3C"
                                                           radius: 4
                                                           Text {
                                                               anchors.centerIn: parent
                                                               text: armController.shoulderAngle + "°"
                                                               color: "white"
                                                               font.bold: true
                                                               font.pointSize: 10
                                                           }
                                                       }

                                                       Rectangle {
                                                           width: 60; height: 25
                                                           color: "#F39C12"
                                                           radius: 4
                                                           Text {
                                                               anchors.centerIn: parent
                                                               text: armController.elbowAngle + "°"
                                                               color: "white"
                                                               font.bold: true
                                                               font.pointSize: 10
                                                           }
                                                       }

                                                       Rectangle {
                                                           width: 60; height: 25
                                                           color: "#27AE60"
                                                           radius: 4
                                                           Text {
                                                               anchors.centerIn: parent
                                                               text: armController.wristAngle + "°"
                                                               color: "white"
                                                               font.bold: true
                                                               font.pointSize: 10
                                                           }
                                                       }
                                                   }
                                               }
                                           }

                                           // Manual Servo Control
                                           Rectangle {
                                               Layout.fillWidth: true
                                               height: 60
                                               color: "white"
                                               radius: 8
                                               border.color: "#BDC3C7"
                                               border.width: 1

                                               RowLayout {
                                                   anchors.fill: parent
                                                   anchors.margins: 10
                                                   spacing: 10

                                                   Text {
                                                       text: "Quick Actions:"
                                                       font.bold: true
                                                       font.pointSize: 10
                                                       color: "#34495E"
                                                   }

                                                   Button {
                                                       text: "Grab Position"
                                                       font.pointSize: 9
                                                       onClicked: {
                                                           armController.moveToPosition(20, 0, 5)
                                                           keyboardHandler.forceActiveFocus()
                                                       }
                                                       enabled: armController.isConnected
                                                   }

                                                   Button {
                                                       text: "Release Position"
                                                       font.pointSize: 9
                                                       onClicked: {
                                                           armController.moveToPosition(15, 0, 20)
                                                           keyboardHandler.forceActiveFocus()
                                                       }
                                                       enabled: armController.isConnected
                                                   }

                                                   Button {
                                                       text: "Rest Position"
                                                       font.pointSize: 9
                                                       onClicked: {
                                                           armController.moveToPosition(10, 0, 25)
                                                           keyboardHandler.forceActiveFocus()
                                                       }
                                                       enabled: armController.isConnected
                                                   }
                                               }
                                           }
                                       }
                                   }

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
                                                keyboardHandler.forceActiveFocus()
                                            }
                                            cursorShape: Qt.PointingHandCursor
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                Layout.preferredHeight: 0
                                color: "transparent"

                                Slider {
                                    id: verticalSlider
                                    orientation: Qt.Vertical
                                    anchors.bottom: parent.bottom
                                    anchors.left: parent.left
                                    anchors.margins: 20
                                    anchors.bottomMargin: 20
                                    height: 400
                                    width: 30
                                    from: -255
                                    to: 255
                                    value: 0

                                    Behavior on value {
                                        NumberAnimation {
                                            duration: 2000
                                            easing.type: Easing.OutQuad
                                        }
                                    }

                                    onValueChanged: {
                                        carController.speedValue = Math.round(value)
                                    }

                                    onPressedChanged: {
                                        carController.setSpeedPressed(pressed)

                                        if (!pressed && keyboardHandler.autoReturnEnabled && Math.abs(value) <= carController.speedDeadZone) {
                                            value = 0
                                        }

                                        if (!pressed) {
                                            keyboardHandler.forceActiveFocus()
                                        }
                                    }

                                    background: Rectangle {
                                        anchors.fill: parent
                                        implicitWidth: 4
                                        color: "darkgrey"
                                        radius: 10

                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: parent.width + 10
                                            height: (carController.speedDeadZone * 2 / 510) * parent.height
                                            color: "lightcoral"
                                            opacity: 0.3
                                            radius: 5
                                        }
                                    }

                                    handle: Rectangle {
                                        x: verticalSlider.leftPadding + (verticalSlider.horizontal ? verticalSlider.visualPosition * (verticalSlider.availableWidth - width) : (verticalSlider.availableWidth - width) / 2)
                                        y: verticalSlider.topPadding + (verticalSlider.horizontal ? (verticalSlider.availableHeight - height) / 2 : verticalSlider.visualPosition * (verticalSlider.availableHeight - height))
                                        implicitWidth: 50
                                        implicitHeight: 50
                                        radius: 25
                                        color: Math.abs(verticalSlider.value) > carController.speedDeadZone ? "#3E5F44" : "#8E8E8E"
                                        border.color: "#bdbebf"
                                    }

                                    Text {
                                        anchors.left: parent.right
                                        anchors.leftMargin: 25
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: "Speed: " + Math.round(verticalSlider.value)
                                        color: "black"
                                        font.pointSize: 14
                                    }
                                }

                                Slider {
                                    id: horizontalSlider
                                    orientation: Qt.Horizontal
                                    anchors.bottom: parent.bottom
                                    anchors.right: parent.right
                                    anchors.margins: 20
                                    anchors.bottomMargin: 80
                                    width: 250
                                    height: 30
                                    from: -50
                                    to: 50
                                    value: carController.turnValue

                                    Behavior on value {
                                        NumberAnimation {
                                            duration: 700
                                            easing.type: Easing.OutQuad
                                        }
                                    }

                                    onValueChanged: {
                                        if (pressed) {
                                            carController.turnValue = Math.round(value)
                                        } else {
                                            carController.turnValue = Math.round(value)
                                        }
                                    }

                                    onPressedChanged: {
                                        carController.setSteeringPressed(pressed)

                                        if (!pressed) {
                                            if (!keyboardHandler.aPressed && !keyboardHandler.dPressed) {
                                                value = 0
                                            } else if (keyboardHandler.autoTurnEnabled && Math.abs(value) <= carController.turnDeadZone) {
                                                value = 0
                                            }

                                            keyboardHandler.forceActiveFocus()
                                        }
                                    }

                                    background: Rectangle {
                                        anchors.fill: parent
                                        implicitWidth: 4
                                        color: "darkgrey"
                                        radius: 10

                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: (carController.turnDeadZone * 2 / 100) * parent.width
                                            height: parent.height + 10
                                            color: "lightcoral"
                                            opacity: 0.3
                                            radius: 5
                                        }
                                    }

                                    handle: Rectangle {
                                        x: horizontalSlider.leftPadding + (horizontalSlider.horizontal ? horizontalSlider.visualPosition * (horizontalSlider.availableWidth - width) : (horizontalSlider.availableWidth - width) / 2)
                                        y: horizontalSlider.topPadding + (horizontalSlider.horizontal ? (horizontalSlider.availableHeight - height) / 2 : horizontalSlider.visualPosition * (horizontalSlider.availableHeight - height))
                                        implicitWidth: 50
                                        implicitHeight: 50
                                        radius: 25
                                        color: Math.abs(horizontalSlider.value) > carController.turnDeadZone ? "#3E5F44" : "#8E8E8E"
                                        border.color: "#bdbebf"
                                    }

                                    Text {
                                        anchors.top: parent.bottom
                                        anchors.topMargin: 20
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        text: "Turn: " + Math.round(horizontalSlider.value)
                                        color: "black"
                                        font.pointSize: 16
                                    }
                                }

                                Rectangle {
                                    anchors.bottom: parent.bottom
                                    anchors.left: parent.left
                                    anchors.bottomMargin: 30
                                    anchors.leftMargin: 80
                                    width: 200
                                    height: 100
                                    color: "white"
                                    opacity: 0.9
                                    radius: 5
                                    border.color: "grey"

                                    Column {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 5

                                        Text {
                                            text: "Motor Commands"
                                            font.bold: true
                                            font.pointSize: 12
                                        }

                                        Text {
                                            text: "Left: " + carController.leftMotorSpeed
                                            font.pointSize: 10
                                        }

                                        Text {
                                            text: "Right: " + carController.rightMotorSpeed
                                            font.pointSize: 10
                                        }

                                        Text {
                                            text: "Status: " + (carController.speedValue === 0 && carController.turnValue === 0 ? "Stopped" : "Moving")
                                            font.pointSize: 10
                                            color: carController.speedValue === 0 && carController.turnValue === 0 ? "red" : "green"
                                        }
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
                                keyboardHandler.forceActiveFocus()
                            }
                            Layout.alignment: Qt.AlignVCenter
                        }
                    }

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
