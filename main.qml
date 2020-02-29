import QtQuick 2.13
import QtQuick.Controls 2.13
import QtQuick.Layouts 1.3
import Qt.labs.platform 1.0

ApplicationWindow {
    id: window
    visible: true
    visibility: "Maximized"
    width: 1280
    height: 720
    title: qsTr("wifi heat map")

    readonly property int minSidebarWidth: 400
    readonly property int sidebarWidth: window.width / 4
                                        < minSidebarWidth ? minSidebarWidth : window.width / 4
    readonly property bool inPortrait: window.width < window.height
    readonly property bool sidebarDisabled: inPortrait | window.width < 3 * minSidebarWidth

    FileDialog {
        id: fileDialog
        title: "Choose an image"
        onAccepted: heatmap.sourceBackground = file
        nameFilters: ["Image files (*.png *.jpg)"]
    }

    //HACK: https://stackoverflow.com/questions/24927850/get-the-path-from-a-qml-url
    function urlToPath(urlString) {
        var s

        if (urlString.startsWith("file:///")) {
            var k = urlString.charAt(9) === ':' ? 8 : 7
            s = urlString.substring(k)
        } else {
            s = urlString
        }
        return decodeURIComponent(s)
    }

    FileDialog {
        id: exportImageDialog
        title: "Export Image"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "png"
        onAccepted: {
            var path = urlToPath(file.toString())
            workingArea.grabToImage(function (result) {
                result.saveToFile(path)
            })
        }
        nameFilters: ["Image files (*.png *.jpg)"]
    }

    FileDialog {
        id: saveFileDialog
        title: "save file"
        fileMode: FileDialog.SaveFile
        defaultSuffix: "json"
        onAccepted: {
            document.save(file)
        }
        nameFilters: ["json files (*.json)"]
    }

    FileDialog {
        id: loadFileDialog
        title: "load file"
        onAccepted: {
            document.load(file)
        }
        nameFilters: ["json files (*.json)"]
    }

    function update_heatmap() {
        heatmap.sourceHeatMap = "image://heatmap/heatmap/" + Math.random()
    }

    Connections {
        target: posModel
        onHeatMapChanged: update_heatmap()
    }

    MessageDialog {
        id: messageDialog
    }

    Rectangle {
        id: body
        anchors.fill: parent
        anchors.leftMargin: !sidebarDisabled ? sidebar.width : undefined
        Rectangle {
            id: workingArea
            width: heatmap.width
            height: heatmap.height
            border.color: "black"
            border.width: 1
            antialiasing: true
            x: parent.width / 2 - width / 2
            y: parent.height / 2 - height / 2
            scale: Math.min(parent.width / width, parent.height / height)
            Connections {
                target: heatmap
                function onBackgroundChanged() {
                    workingArea.scale = Math.min(
                                workingArea.parent.width / width,
                                workingArea.parent.height / height)
                }
            }

            HeatMap {
                id: heatmap
                sourceBackground: "A4_120dpi.png"
                sourceHeatMap: "image://heatmap/heatmap/0"

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (triggerScan.running) {
                            posModel.measure(Qt.point(mouse.x, mouse.y))
                            update_heatmap()
                        } else {
                            messageDialog.text = "Scan process is not running!"
                            messageDialog.visible = true
                        }
                    }
                }
            }

            PinchArea {
                anchors.fill: parent
                pinch.target: workingArea
                pinch.minimumScale: 0.2
                pinch.maximumScale: 10
                pinch.dragAxis: Pinch.XAndYAxis

                //onPinchStarted: workingArea.transformOriginPoint = mapToItem(pinch.center)
                //onPinchFinished: workingArea.transformOriginPoint = workingArea.center
                MouseArea {
                    anchors.fill: parent
                    drag.target: workingArea
                    propagateComposedEvents: true
                    scrollGestureEnabled: false
                    onWheel: {
                        var delta = workingArea.scale * wheel.angleDelta.y / 120 / 10
                        if (parent.pinch.minimumScale > workingArea.scale + delta)
                            return
                        if (parent.pinch.maximumScale < workingArea.scale + delta)
                            return
                        workingArea.scale += delta
                    }
                    onClicked: {
                        mouse.accepted = false
                    }
                }
            }

            Item {
                id: heatmapmarker
                anchors.centerIn: heatmap
                height: heatmap.height
                width: heatmap.width

                Repeater {
                    model: posModel
                    onItemRemoved: update_heatmap()
                    delegate: Marker {
                        width: 20
                        height: 20
                    }
                }
            }
        }
    }

    ToolButton {
        id: toolButton
        text: "\u2630"
        font.pixelSize: Qt.application.font.pixelSize * 1.6
        onClicked: sidebar.open()
    }

    Rectangle {
        color: "#FFFFFF"
        id: legend
        width: 350
        height: 30
        anchors.rightMargin: 5
        anchors.bottomMargin: 5
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        Image {
            anchors.fill: parent
            sourceSize.width: parent.width
            sourceSize.height: parent.height
            source: "image://heatmap/legend/0"
            opacity: 0.5
        }

        RowLayout {
            anchors.fill: parent
            Text {
                text: "-80\u2009dbm"
                Layout.alignment: Qt.AlignLeft
            }
            Text {
                text: "-67\u2009dbm"
                Layout.alignment: Qt.AlignCenter
            }
            Text {
                Layout.alignment: Qt.AlignRight
                text: "-54\u2009dbm"
            }
        }
    }

    Drawer {
        id: sidebar
        width: sidebarWidth
        height: window.height

        modal: sidebarDisabled
        interactive: sidebarDisabled
        position: sidebarDisabled ? 0 : 1
        visible: !sidebarDisabled

        ColumnLayout {
            anchors.fill: parent
            Button {
                text: "load map image"
                onClicked: {
                    fileDialog.open()
                }
                Layout.fillWidth: true
            }

            Button {
                text: triggerScan.running ? "stop scan process" : "start scan process"
                onClicked: triggerScan.start_scanner()
                Layout.fillWidth: true
            }

            Button {
                text: "save"
                onClicked: saveFileDialog.open()
                Layout.fillWidth: true
            }

            Button {
                text: "load"
                onClicked: loadFileDialog.open()
                Layout.fillWidth: true
            }

            Button {
                text: "export image"
                onClicked: exportImageDialog.open()
                Layout.fillWidth: true
            }

            ComboBox {
                id: interfaceComboBox
                model: interfaceModel
                textRole: "name"
                Layout.fillWidth: true

                Binding {
                    target: interfaceModel
                    property: "currentIndex"
                    value: interfaceComboBox.currentIndex
                }
            }

            BssTable {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                ScrollBar.vertical: ScrollBar {}
                ScrollBar.horizontal: ScrollBar {}
            }
        }
    }
}
