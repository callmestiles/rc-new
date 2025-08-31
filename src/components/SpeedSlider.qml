import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

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

    Component.onCompleted: {
        // Initialize the slider with the current car controller value
        value = carController.speedValue
    }

    // Listen to changes from the CarController (hardware updates)
    Connections {
        target: carController
        function onSpeedValueChanged() {
            // Only update the slider if the user isn't currently interacting with it
            if (!verticalSlider.pressed) {
                console.log("Updating slider from hardware:", carController.speedValue)
                verticalSlider.value = carController.speedValue
            }
        }
    }

    onMoved: {
        // Use onMoved instead of onValueChanged to only respond to user interaction
        carController.speedValue = Math.round(value)
    }

    onPressedChanged: {
        carController.setSpeedPressed(pressed)
        if (!pressed) {
            // When user releases, make sure we're in sync
            carController.speedValue = Math.round(value)
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
        implicitWidth: 70
        implicitHeight: 70
        radius: 35
        color: Math.abs(verticalSlider.value) > carController.speedDeadZone ? "#3E5F44" : "#8E8E8E"
        border.color: "#bdbebf"
    }

    Text {
        anchors.left: parent.right
        anchors.leftMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        text: Math.round(verticalSlider.value)
        color: "black"
        font.pointSize: 14
    }
}
