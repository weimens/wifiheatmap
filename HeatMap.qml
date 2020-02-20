import QtQuick 2.0

Item {
    id: heatmap
    property alias sourceBackground: mapimage.source
    property alias sourceHeatMap: heatmapimage.source

    Image {
        id: mapimage
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
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
