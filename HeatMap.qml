import QtQuick 2.13

Item {
    id: heatmap
    property alias sourceBackground: mapimage.source
    property alias sourceHeatMap: heatmapimage.source

    Connections {
        target: document
        onMapImageChanged: mapimage.reload()
    }

    Image {
        id: mapimage
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
        cache: false
        function reload() {
            var oldSource = source
            source = ""
            source = oldSource
        }
    }

    Binding {
        target: heatmap
        property: "width"
        value: mapimage.paintedWidth
    }

    Binding {
        target: heatmap
        property: "height"
        value: mapimage.paintedHeight
    }

    Image {
        id: heatmapimage
        anchors.centerIn: mapimage

        Binding {
            target: heatmapimage
            property: "width"
            value: mapimage.paintedWidth
        }

        Binding {
            target: heatmapimage
            property: "height"
            value: mapimage.paintedHeight
        }

        opacity: 0.5
        sourceSize.width: width
        sourceSize.height: height
    }
}
