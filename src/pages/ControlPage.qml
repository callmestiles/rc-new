import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
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
                        ArmControlPanel {}

                        //MiniMap
                        // MiniMap {}
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
