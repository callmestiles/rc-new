import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: armPanel
    width: 400
    height: 300
    color: "#f0f0f0"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        Text {
            text: "Arm Control"
            font.pixelSize: 18
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        GridLayout {
            columns: 2
            columnSpacing: 20
            rowSpacing: 10
            Layout.alignment: Qt.AlignHCenter

            Text { text: "Base:" }
            TextField {
                text: armController.baseAngle.toString()
                placeholderText: "0-180"
                validator: IntValidator { bottom: 0; top: 180 }
                onEditingFinished: {
                    let angle = parseInt(text)
                    if (!isNaN(angle) && angle >= 0 && angle <= 180) {
                        armController.moveBase(angle)
                    }
                }
            }

            Text { text: "Shoulder:" }
            TextField {
                text: armController.shoulderAngle.toString()
                placeholderText: "0-180"
                validator: IntValidator { bottom: 0; top: 180 }
                onEditingFinished: {
                    let angle = parseInt(text)
                    if (!isNaN(angle) && angle >= 0 && angle <= 180) {
                        armController.moveShoulder(angle)
                    }
                }
            }

            Text { text: "Elbow:" }
            TextField {
                text: armController.elbowAngle.toString()
                placeholderText: "0-180"
                validator: IntValidator { bottom: 0; top: 180 }
                onEditingFinished: {
                    let angle = parseInt(text)
                    if (!isNaN(angle) && angle >= 0 && angle <= 180) {
                        armController.moveElbow(angle)
                    }
                }
            }

            Text { text: "Wrist:" }
            TextField {
                text: armController.wristAngle.toString()
                placeholderText: "0-180"
                validator: IntValidator { bottom: 0; top: 180 }
                onEditingFinished: {
                    let angle = parseInt(text)
                    if (!isNaN(angle) && angle >= 0 && angle <= 180) {
                        armController.moveWrist(angle)
                    }
                }
            }

            Text { text: "Gripper:" }
            TextField {
                text: armController.gripperAngle.toString()
                placeholderText: "0-180"
                validator: IntValidator { bottom: 0; top: 180 }
                onEditingFinished: {
                    let angle = parseInt(text)
                    if (!isNaN(angle) && angle >= 0 && angle <= 180) {
                        armController.moveGripper(angle)
                    }
                }
            }
        }

        Text {
            id: statusText
            text: "Ready"
            font.pixelSize: 12
            color: "gray"
            Layout.alignment: Qt.AlignHCenter
        }
    }

    Connections {
        target: armController
        function onCommandSent(command) {
            statusText.text = "Sent: " + command
            statusText.color = "green"
        }
        function onNetworkError(error) {
            statusText.text = "Error: " + error
            statusText.color = "red"
        }
    }
}
