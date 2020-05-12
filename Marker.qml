import QtQuick 2.12

Item {
    id: marker
    Rectangle {
        x: -parent.width / 2
        y: -parent.height / 2
        width: parent.width
        height: parent.height

        color: model.state ? "#cce6f5" : "#cdf5cc"
        transform: Rotation {
            id: scanning
            origin.x: width / 2
            origin.y: height / 2
            angle: marker.seconds * 90
            Behavior on angle {
                SpringAnimation {
                    spring: 2
                    damping: 0.2
                    modulus: 360
                }
            }
        }

        MouseArea {
            id: markerarea
            anchors.fill: parent
            drag.target: marker
            onDoubleClicked: if (model.state)
                                 posModel.remove(index)
            drag.onActiveChanged: {
                if (!markerarea.drag.active) {
                    pos = Qt.point(marker.x, marker.y)
                }
            }
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
    }

    property int seconds

    function timeChanged() {
        var date = new Date
        seconds = date.getUTCSeconds()
    }

    Timer {
        interval: 100
        running: !model.state
        repeat: true
        onRunningChanged: if (!running) {
                              seconds = 0
                          }
        onTriggered: marker.timeChanged()
    }

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
}
