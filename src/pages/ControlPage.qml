import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtMultimedia

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


                        CameraStream {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            streamUrl: "http://192.168.4.1/stream"
                            autoConnect: true
                            showControls: true
                            showStatus: true

                            onFrameReceived: function(pixmap) {
                                console.log("New frame received for processing")
                                // Process frame for object detection here
                                // processFrameForObjectDetection(pixmap)
                            }

                            onConnectionChanged: function(connected) {
                                if (connected) {
                                    console.log("Camera connected successfully")
                                } else {
                                    console.log("Camera disconnected")
                                }
                            }

                            onErrorOccurred: function(error) {
                                console.log("Camera error:", error)
                                // Show error dialog or notification
                            }
                        }

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
