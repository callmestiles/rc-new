import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    anchors.fill: parent
    color: "blue"

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
            implicitWidth: 70
            implicitHeight: 70
            radius: 35
            color: Math.abs(verticalSlider.value) > carController.speedDeadZone ? "#3E5F44" : "#8E8E8E"
            border.color: "#bdbebf"
        }

        Text {
            anchors.left: parent.right
            anchors.leftMargin: 25
            anchors.verticalCenter: parent.verticalCenter
            text: Math.round(verticalSlider.value)
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
        anchors.bottomMargin: 30
        width: 300
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
            // Only update the controller when the user is actively dragging
            if (pressed) {
                carController.turnValue = Math.round(value)
            }
        }

        onPressedChanged: {
            carController.setSteeringPressed(pressed)

            if (pressed) {
                // When user starts dragging, update the controller with current value
                carController.turnValue = Math.round(value)
            }
        }

        // Make sure the slider follows the controller's value changes
        Connections {
            target: carController
            function onTurnValueChanged() {
                if (!horizontalSlider.pressed) {
                    horizontalSlider.value = carController.turnValue
                }
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
            implicitWidth: 70
            implicitHeight: 70
            radius: 35
            color: Math.abs(horizontalSlider.value) > carController.turnDeadZone ? "#3E5F44" : "#8E8E8E"
            border.color: "#bdbebf"
        }

        Text {
            anchors.bottom: parent.top
            anchors.bottomMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            text: Math.round(horizontalSlider.value)
            color: "black"
            font.pointSize: 16
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.bottomMargin: 30
        anchors.leftMargin: 80
        width: 160
        height: 110
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
