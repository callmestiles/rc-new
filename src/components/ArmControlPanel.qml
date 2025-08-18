import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

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
                        }
                    }

                    Button {
                        text: "Apply Position"
                        onClicked: {
                            armController.moveToPosition(xSlider.value, ySlider.value, zSlider.value)
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
                    }
                    enabled: armController.isConnected
                }

                Button {
                    text: "Release Position"
                    font.pointSize: 9
                    onClicked: {
                        armController.moveToPosition(15, 0, 20)
                    }
                    enabled: armController.isConnected
                }

                Button {
                    text: "Rest Position"
                    font.pointSize: 9
                    onClicked: {
                        armController.moveToPosition(10, 0, 25)
                    }
                    enabled: armController.isConnected
                }
            }
        }
    }
}
