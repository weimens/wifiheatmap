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
        onAccepted: image.source = fileUrl
        nameFilters: ["Image files (*.png *.jpg)"]
        selectedNameFilter: "Image files (*.png *.jpg)"
    }

    function update_heatmap() {
        heatmapimage.source = "image://heatmap/heatmap/" + Math.random()
    }

    Connections {
        target: posModel
        onHeatMapChanged: update_heatmap()
    }

    Rectangle {
        id: pane
        width: parent.width
        height: parent.height
        border.color: "black"
        border.width: 1
        antialiasing: true

        Image {
            id: image
            anchors.centerIn: parent
            fillMode: Image.PreserveAspectFit
            source: "A4_120dpi.png"
            Component.onCompleted: {
                parent.width = width
                parent.height = height
            }
        }

        Image {
            id: heatmapimage
            anchors.centerIn: image

            Binding {
                target: heatmapimage
                property: "width"
                value: image.paintedWidth
            }

            Binding {
                target: heatmapimage
                property: "height"
                value: image.paintedHeight
            }

            opacity: 0.5
            sourceSize.width: width
            sourceSize.height: height
            source: "image://heatmap/heatmap/0"
        }

        Item {
            id: heatmap
            anchors.centerIn: image

            Binding {
                target: heatmap
                property: "width"
                value: image.paintedWidth
            }

            Binding {
                target: heatmap
                property: "height"
                value: image.paintedHeight
            }

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
            anchors.centerIn: image
            height: image.paintedHeight
            width: image.paintedWidth

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
