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
    value: 0

    // Behavior on value {
    //     NumberAnimation {
    //         duration: 2000
    //         easing.type: Easing.OutQuad
    //     }
    // }

    onValueChanged: {
        carController.speedValue = Math.round(value)
    }

    onPressedChanged: {
        carController.setSpeedPressed(pressed)
    }

    Connections {
        target: carController
        function onSpeedValueChanged(){
            verticalSlider.value = carController.speedValue
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
