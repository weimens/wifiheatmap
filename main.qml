import QtQuick 2.14
import QtQuick.Controls 2.14

ApplicationWindow {
    id: window
    visible: true
    visibility: "Maximized"
    width: 640
    height: 480
    title: qsTr("wifi heat map")

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

        Item {
            id: heatmap
            anchors.centerIn: image
            height: image.paintedHeight
            width: image.paintedWidth

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    posModel.append({
                                         "pos": Qt.point(mouse.x, mouse.y),
                                         "z": -50
                                     })
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
                        text: Math.round(model.z * (-1))
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
                            }
                        }
                    }
                }
            }
        }
    }
}
