import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.3

ApplicationWindow {
    id: window
    visible: true
    visibility: "Maximized"
    width: 640
    height: 480
    title: qsTr("wifi heat map")

    FileDialog {
        id: fileDialog
        title: "Choose an image"
        onAccepted: heatmap.sourceBackground = fileUrl
        nameFilters: ["Image files (*.png *.jpg)"]
        selectedNameFilter: "Image files (*.png *.jpg)"
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
        selectExisting: false
        onAccepted: {
            var path = urlToPath(fileUrl)
            console.debug(path)
            pane.grabToImage(function (result) {
                console.debug(result.saveToFile(path))
            })
        }
        nameFilters: ["Image files (*.png *.jpg)"]
        selectedNameFilter: "Image files (*.png *.jpg)"
    }

    FileDialog {
        id: saveFileDialog
        title: "save file"
        selectExisting: false
        onAccepted: {
            document.save(fileUrl)
        }
        nameFilters: ["Image files (*.json)"]
        selectedNameFilter: "Image files (*.json)"
    }

    FileDialog {
        id: loadFileDialog
        title: "load file"
        onAccepted: {
            document.load(fileUrl)
        }
        nameFilters: ["Image files (*.json)"]
        selectedNameFilter: "Image files (*.json)"
    }

    function update_heatmap() {
        heatmap.sourceHeatMap = "image://heatmap/heatmap/" + Math.random()
    }

    Connections {
        target: posModel
        onHeatMapChanged: update_heatmap()
    }

    Rectangle {
        id: pane
        width: heatmap.width
        height: heatmap.height
        border.color: "black"
        border.width: 1
        antialiasing: true

        HeatMap {
            id: heatmap
            sourceBackground: "A4_120dpi.png"
            sourceHeatMap: "image://heatmap/heatmap/0"

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    posModel.measure(Qt.point(mouse.x, mouse.y))
                    update_heatmap()
                }
            }
        }

        PinchArea {
            anchors.fill: parent
            pinch.target: pane
            pinch.minimumScale: 0.2
            pinch.maximumScale: 10
            pinch.dragAxis: Pinch.XAndYAxis

            //onPinchStarted: pane.transformOriginPoint = mapToItem(pinch.center)
            //onPinchFinished: pane.transformOriginPoint = pane.center
            MouseArea {
                anchors.fill: parent
                drag.target: pane
                propagateComposedEvents: true
                scrollGestureEnabled: false
                onWheel: {
                    var delta = pane.scale * wheel.angleDelta.y / 120 / 10
                    if (parent.pinch.minimumScale > pane.scale + delta)
                        return
                    if (parent.pinch.maximumScale < pane.scale + delta)
                        return
                    pane.scale += delta
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

                Rectangle {
                    id: marker
                    width: 20
                    height: 20
                    color: "#cce6f5"
                    Binding {
                        target: marker
                        property: "x"
                        value: pos.x
                    }
                    Binding {
                        target: marker
                        property: "y"
                        value: pos.y
                    }

                    Text {
                        text: {
                            if (isNaN(model.z))
                                return ""
                            return Math.round(model.z * (-1))
                        }
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    MouseArea {
                        id: markerarea
                        anchors.fill: parent
                        drag.target: marker
                        onDoubleClicked: posModel.remove(index)
                        drag.onActiveChanged: {
                            if (!markerarea.drag.active) {
                                pos = Qt.point(marker.x, marker.y)
                                update_heatmap()
                            }
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        width: 400
        Button {
            text: "load map image"
            onClicked: {
                fileDialog.open()
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Button {
            text: "export image"
            onClicked: exportImageDialog.open()
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Button {
            text: "save"
            onClicked: saveFileDialog.open()
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Button {
            text: "load"
            onClicked: loadFileDialog.open()
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        ComboBox {
            id: interfaceComboBox
            model: interfaceModel
            textRole: "name"
            valueRole: "interface_index"
            Layout.fillWidth: true
            Layout.fillHeight: true

            Binding {
                target: posModel
                property: "interface_index"
                value: interfaceComboBox.currentValue
            }
        }

        Rectangle {
            color: "#FFFFFF"
            height: 200
            Layout.fillWidth: true
            Layout.fillHeight: true
            ListView {
                anchors.fill: parent
                model: bssmodel
                delegate: CheckDelegate {
                    width: parent.width
                    text: ssid + ' (' + bss + " ch: " + channel + " freq: " + freq + ')'
                    checked: selected
                    onCheckStateChanged: selected = checked
                }
                clip: true
                ScrollBar.vertical: ScrollBar {}
                ScrollBar.horizontal: ScrollBar {}
            }
        }
    }
}
