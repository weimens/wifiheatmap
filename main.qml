import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import Qt.labs.platform 1.0 as Labs

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

    Labs.MessageDialog {
        buttons: Labs.MessageDialog.Save | Labs.MessageDialog.Cancel | Labs.MessageDialog.Discard
        id: messageDialogQuit
        text: "Save before closing?"
        onDiscardClicked: Qt.quit()
        onSaveClicked: {
            saveCloseFileDialog.open()
        }
    }

    Labs.MessageDialog {
        buttons: Labs.MessageDialog.Save | Labs.MessageDialog.Cancel | Labs.MessageDialog.No
        id: messageDialogNew
        text: "Save file before open a new one?"
        onNoClicked: document.newDocument()
        onSaveClicked: saveNewFileDialog.open()
    }

    onClosing: {
        close.accepted = !document.needsSaving
        onTriggered: if (!close.accepted) {
            messageDialogQuit.open()
        }
    }

    menuBar: MenuBar {
        id: menuBar
        Menu {
            title: qsTr("File")
            Action {
                text: qsTr("New")
                onTriggered: {
                    if (!document.needsSaving) {
                        document.newDocument()
                    } else {
                        messageDialogNew.open()
                    }
                }
            }
            Action {
                text: qsTr("Open")
                onTriggered: openFileDialog.open()
            }
            Action {
                text: document.needsSaving ? qsTr("Save*") : qsTr("Save")
                onTriggered: saveFileDialog.open()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Load Foor Plan")
                onTriggered: floorPlanFileDialog.open()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Export Image")
                onTriggered: exportImageDialog.open()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Quit")
                onTriggered: window.close()
            }
        }
    }

    Labs.FileDialog {
        id: floorPlanFileDialog
        title: "Choose an image"
        onAccepted: document.mapImageUrl = file
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

    Labs.FileDialog {
        id: exportImageDialog
        title: "Export Image"
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: "png"
        onAccepted: {
            var path = urlToPath(file.toString())
            workingArea.grabToImage(function (result) {
                result.saveToFile(path)
            })
        }
        nameFilters: ["Image files (*.png *.jpg)"]
    }

    Labs.FileDialog {
        id: saveFileDialog
        title: "save file"
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: "whmap"
        onAccepted: {
            document.save(file)
        }
        nameFilters: ["whmap files (*.whmap)"]
    }

    Labs.FileDialog {
        //FIXME: nearly the same as saveFileDialog
        id: saveCloseFileDialog
        title: "save file"
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: "whmap"
        onAccepted: {
            if (document.save(file)) {
                Qt.quit()
            }
        }
        nameFilters: ["whmap files (*.whmap)"]
    }

    Labs.FileDialog {
        //FIXME: nearly the same as saveFileDialog
        id: saveNewFileDialog
        title: "save file"
        fileMode: Labs.FileDialog.SaveFile
        defaultSuffix: "whmap"
        onAccepted: {
            if (document.save(file)) {
                document.newDocument()
            }
        }
        nameFilters: ["whmap files (*.whmap)"]
    }

    Labs.FileDialog {
        id: openFileDialog
        title: "open file"
        onAccepted: {
            document.load(file)
        }
        nameFilters: ["whmap files (*.whmap)"]
    }

    Labs.MessageDialog {
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

            HeatMap {
                id: heatmap
                sourceBackground: "image://document/mapimage"
                sourceHeatMap: "image://heatmap/heatmap"

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (Qt.platform.os == "linux") {
                            if (linuxScan.running) {
                                linuxScan.measure(Qt.point(mouse.x, mouse.y))
                            } else {
                                messageDialog.text = "Scan process is not running!"
                                messageDialog.visible = true
                            }
                        }
                        if (Qt.platform.os == "android") {
                            androidScan.measure(Qt.point(mouse.x, mouse.y))
                        }
                        if (Qt.platform.os == "windows") {
                            windowsScan.measure(Qt.point(mouse.x, mouse.y))
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
        text: "\u2261"
        font.pixelSize: Qt.application.font.pixelSize * 1.6
        onClicked: sidebar.open()
    }

    Button {
        id: legend
        width: 350
        height: 30
        anchors.rightMargin: 5
        anchors.bottomMargin: 5
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        onClicked: popup.open()

        Image {
            anchors.fill: parent
            sourceSize.width: parent.width
            sourceSize.height: parent.height
            source: "image://heatmap/legend/0"
            opacity: 0.5
        }

        Text {
            text: heatMapCalc.zmin + "\u2009dbm"
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: (heatMapCalc.zmin + heatMapCalc.zmax) / 2 + "\u2009dbm"
            anchors.centerIn: parent
        }
        Text {
            text: heatMapCalc.zmax + "\u2009dbm"
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
        }

        Popup {

            id: popup
            padding: 0

            parent: Overlay.overlay

            x: parent.width - legend.anchors.rightMargin - width
            y: parent.height - legend.anchors.bottomMargin - height
            width: legend.width
            height: legend.height

            enter: Transition {
                NumberAnimation {
                    property: "height"
                    duration: 0
                    from: legend.height
                    to: zminInput.height
                }
            }
            exit: Transition {
                NumberAnimation {
                    property: "height"
                    duration: 0
                    from: zminInput.height
                    to: legend.height
                }
            }

            Image {
                anchors.fill: parent
                sourceSize.width: parent.width
                sourceSize.height: parent.height
                source: "image://heatmap/legend/0"
                opacity: 0.5
            }

            Row {
                anchors.left: parent.left
                Slider {
                    id: zminInput
                    value: heatMapCalc.zmin
                    from: -100
                    to: 0
                    stepSize: 1
                    orientation: Qt.Vertical
                    Binding {
                        target: heatMapCalc
                        property: "zmin"
                        value: zminInput.value
                    }
                }
                Text {
                    text: heatMapCalc.zmin + "\u2009dbm"
                    anchors.top: parent.top
                }
            }
            Text {
                id: zmidText
                text: (heatMapCalc.zmin + heatMapCalc.zmax) / 2 + "\u2009dbm"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Row {
                anchors.right: parent.right

                Text {
                    text: heatMapCalc.zmax + "\u2009dbm"
                    anchors.top: parent.top
                }
                Slider {
                    id: zmaxInput
                    value: heatMapCalc.zmax
                    from: -100
                    to: 0
                    stepSize: 1
                    orientation: Qt.Vertical
                    Binding {
                        target: heatMapCalc
                        property: "zmax"
                        value: zmaxInput.value
                    }
                }
            }
        }
    }

    Drawer {
        id: sidebar
        width: sidebarWidth
        y: menuBar.height
        height: window.height - menuBar.height

        modal: sidebarDisabled
        interactive: sidebarDisabled
        position: sidebarDisabled ? 0 : 1
        visible: !sidebarDisabled

        ColumnLayout {
            anchors.fill: parent

            Button {
                visible: Qt.platform.os == "linux"
                text: Qt.platform.os == "linux"
                      && linuxScan.running ? "stop scan process" : "start scan process"
                onClicked: Qt.platform.os == "linux"
                           && linuxScan.start_scanner()
                Layout.fillWidth: true
                Layout.topMargin: 5
            }

            ComboBox {
                visible: Qt.platform.os == "linux"
                         || Qt.platform.os == "windows"
                id: interfaceComboBox
                model: (Qt.platform.os == "linux"
                        || Qt.platform.os == "windows") ? interfaceModel : undefined
                textRole: "name"
                Layout.fillWidth: true

                Binding {
                    target: interfaceModel //FIXME
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
