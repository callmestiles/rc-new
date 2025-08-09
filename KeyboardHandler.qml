import QtQuick

FocusScope {
    anchors.fill: parent
    focus: true

    // Properties to expose key states
    property bool wPressed: false
    property bool sPressed: false
    property bool aPressed: false
    property bool dPressed: false

    // Properties for auto toggles
    property bool autoReturnEnabled: true
    property bool autoTurnEnabled: true

    // Signal for when keys change
    signal keysChanged()

    Keys.onPressed: function(event) {
        if (event.isAutoRepeat) return

        switch(event.key) {
            case Qt.Key_W:
                wPressed = true
                break
            case Qt.Key_S:
                sPressed = true
                break
            case Qt.Key_A:
                aPressed = true
                break
            case Qt.Key_D:
                dPressed = true
                break
            case Qt.Key_Shift:
                autoReturnEnabled = !autoReturnEnabled
                autoTurnEnabled = autoReturnEnabled
                console.log("Auto center", autoReturnEnabled ? "enabled" : "disabled")
                console.log("Auto turn", autoTurnEnabled ? "enabled" : "disabled")
                break
            case Qt.Key_Control:
                autoTurnEnabled = !autoTurnEnabled
                console.log("Auto turn", autoTurnEnabled ? "enabled" : "disabled")
                break
        }
        keysChanged()
        event.accepted = true
    }

    Keys.onReleased: function(event) {
        if (event.isAutoRepeat) return

        switch(event.key) {
            case Qt.Key_W:
                wPressed = false
                break
            case Qt.Key_S:
                sPressed = false
                break
            case Qt.Key_A:
                aPressed = false
                break
            case Qt.Key_D:
                dPressed = false
                break
        }
        keysChanged()
        event.accepted = true
    }

    Component.onCompleted: {
        forceActiveFocus()
    }
}
