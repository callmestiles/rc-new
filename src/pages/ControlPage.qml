import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import CarController
import ArmController
import "../components"

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

        //replace with StatusBar
        StatusBar {}

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
                        // ArmControlPanel {}

                        //MiniMap
                        MiniMap {}
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredHeight: 0
                    color: "transparent"

                    SpeedSlider {}

                    TurnSlider {}

                    MotorCommands {}

                    StopButton {}
                }
            }
        }
    }
}
