import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CameraStream 1.0

Item {
    id: root

    // Public properties that can be set from outside
    property string streamUrl: "http://192.168.4.1/stream"
    property bool autoConnect: true
    property bool showControls: true
    property bool showStatus: true
    property color backgroundColor: "black"
    property color borderColor: "gray"
    property int borderWidth: 2

    // Read-only properties that external code can bind to
    readonly property bool connected: mjpegStreamer.connected
    readonly property string status: mjpegStreamer.status
    readonly property int frameRate: mjpegStreamer.frameRate
    readonly property alias streamer: mjpegStreamer

    // Signals that external code can connect to
    signal frameReceived(var pixmap)
    signal connectionChanged(bool connected)
    signal errorOccurred(string error)

    // Public functions
    function startStream() {
        mjpegStreamer.startStream()
    }

    function stopStream() {
        mjpegStreamer.stopStream()
    }

    function reconnect() {
        mjpegStreamer.reconnect()
    }

    function setUrl(url) {
        mjpegStreamer.url = url
    }

    // Frame update timer to control refresh rate and reduce flicker
    Timer {
        id: frameUpdateTimer
        interval: 33 // ~30 FPS max to prevent excessive updates
        repeat: false
        onTriggered: {
            if (mjpegStreamer.connected) {
                streamImage.source = ""
                streamImage.source = "image://stream/frame"
            }
        }
    }

    MjpegStreamer {
        id: mjpegStreamer
        url: root.streamUrl

        onNewFrame: function(pixmap) {
            root.frameReceived(pixmap)

            // Only update image if timer is not running (throttle updates)
            if (!frameUpdateTimer.running) {
                frameUpdateTimer.start()
            }
        }

        onConnectedChanged: {
            root.connectionChanged(connected)
            if (!connected && status.indexOf("Error") >= 0) {
                root.errorOccurred(status)
            }

            // Update image immediately on connection change
            if (connected) {
                streamImage.source = "image://stream/frame"
            } else {
                streamImage.source = ""
            }
        }

        onUrlChanged: {
            root.streamUrl = url
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        // Control panel - only visible if showControls is true
        RowLayout {
            Layout.fillWidth: true
            visible: root.showControls

            TextField {
                id: urlField
                Layout.fillWidth: true
                placeholderText: "Camera stream URL..."
                text: root.streamUrl
                onTextChanged: {
                    if (text !== root.streamUrl) {
                        root.streamUrl = text
                        mjpegStreamer.url = text
                    }
                }
            }

            Button {
                text: mjpegStreamer.connected ? "Stop" : "Start"
                onClicked: {
                    if (mjpegStreamer.connected) {
                        mjpegStreamer.stopStream()
                    } else {
                        mjpegStreamer.startStream()
                    }
                }
            }

            Button {
                text: "Reconnect"
                enabled: !mjpegStreamer.connected
                onClicked: mjpegStreamer.reconnect()
            }
        }

        // Status bar - only visible if showStatus is true
        RowLayout {
            Layout.fillWidth: true
            visible: root.showStatus

            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: mjpegStreamer.connected ? "green" : "red"
            }

            Text {
                text: mjpegStreamer.status
                color: mjpegStreamer.connected ? "green" : "red"
                font.pointSize: 9
            }

            Text {
                text: mjpegStreamer.connected ? (mjpegStreamer.frameRate + " FPS") : ""
                color: "blue"
                font.pointSize: 9
                visible: mjpegStreamer.connected
            }

            Item { Layout.fillWidth: true } // Spacer
        }

        // Video display area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: root.backgroundColor
            border.color: root.borderColor
            border.width: root.borderWidth

            Image {
                id: streamImage
                anchors.fill: parent
                anchors.margins: root.borderWidth + 2
                fillMode: Image.PreserveAspectFit
                cache: false
                asynchronous: true
                smooth: true // Enable smooth scaling

                // Use static source to prevent flicker
                source: mjpegStreamer.connected ? "image://stream/frame" : ""

                // Fallback content when no stream
                Rectangle {
                    anchors.fill: parent
                    color: root.backgroundColor
                    visible: !mjpegStreamer.connected

                    Column {
                        anchors.centerIn: parent
                        spacing: 10

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Camera Stream"
                            color: "gray"
                            font.pixelSize: 18
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Not Connected"
                            color: "gray"
                            font.pixelSize: 14
                        }
                    }
                }

                // Loading overlay - only show for first few seconds
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    visible: mjpegStreamer.connected && mjpegStreamer.frameRate === 0

                    Column {
                        anchors.centerIn: parent
                        spacing: 20

                        BusyIndicator {
                            anchors.horizontalCenter: parent.horizontalCenter
                            running: true
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Receiving Stream..."
                            color: "white"
                            font.pixelSize: 16
                        }
                    }

                    // Auto-hide loading after 5 seconds
                    Timer {
                        running: mjpegStreamer.connected && mjpegStreamer.frameRate === 0
                        interval: 5000
                        onTriggered: parent.visible = false
                    }
                }

                // Error overlay
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    visible: mjpegStreamer.status.indexOf("Error") >= 0

                    Text {
                        anchors.centerIn: parent
                        text: "Connection Error"
                        color: "red"
                        font.pixelSize: 16
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        if (root.autoConnect && root.streamUrl !== "") {
            mjpegStreamer.startStream()
        }
    }

    // Clean up when component is destroyed
    Component.onDestruction: {
        mjpegStreamer.stopStream()
    }
}
