import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

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
