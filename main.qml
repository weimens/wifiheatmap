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
        Menu {
            title: "Edit"
            Action {
                text: "undo"
                enabled: undoStack.canUndo
                onTriggered: undoStack.undo()
                shortcut: StandardKey.Undo
            }

            Action {
                text: "redo"
                enabled: undoStack.canRedo
                onTriggered: undoStack.redo()
                shortcut: StandardKey.Redo
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
                        this.forceActiveFocus()
                        controller.measure(Qt.point(mouse.x, mouse.y))
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

    Flow {
        id: flow
        width: window.width - 2 * flow.spacing - sidebarWidth * !sidebarDisabled
        spacing: 5
        layoutDirection: Qt.RightToLeft

        anchors.rightMargin: 5
        anchors.bottomMargin: 5
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        ComboBox {
            id: typeComboBox
            width: 180
            height: 30

            model: typeModel
            textRole: "display"

            currentIndex: typeModel.currentIndex
            Binding {
                target: typeModel
                property: "currentIndex"
                value: typeComboBox.currentIndex
            }
        }

        Button {
            id: legend
            width: 350
            height: 30
            onClicked: popup.opened ? popup.close() : popup.open()

            Image {
                anchors.fill: parent
                sourceSize.width: parent.width
                sourceSize.height: parent.height
                source: "image://heatmap/legend/0"
                opacity: 0.5
            }

            Text {
                text: heatMapLegend.zFormatter(heatMapLegend.zmin)
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                text: heatMapLegend.zFormatter(
                          (heatMapLegend.zmin + heatMapLegend.zmax) / 2)
                anchors.centerIn: parent
            }
            Text {
                text: heatMapLegend.zFormatter(heatMapLegend.zmax)
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
            }

            Popup {
                id: popup
                padding: 0
                y: -popup.height
                width: legend.width
                closePolicy: Popup.CloseOnPressOutsideParent

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
                        value: heatMapLegend.zmin
                        from: heatMapLegend.zfrom
                        to: heatMapLegend.zto
                        stepSize: heatMapLegend.zstepSize
                        orientation: Qt.Vertical
                        Binding {
                            target: heatMapLegend
                            property: "zmin"
                            value: zminInput.value
                        }
                    }
                }

                Row {
                    anchors.right: parent.right
                    Slider {
                        id: zmaxInput
                        value: heatMapLegend.zmax
                        from: heatMapLegend.zfrom
                        to: heatMapLegend.zto
                        stepSize: heatMapLegend.zstepSize
                        orientation: Qt.Vertical
                        Binding {
                            target: heatMapLegend
                            property: "zmax"
                            value: zmaxInput.value
                        }
                    }
                }
            }
        }

        Rectangle {
            height: childrenRect.height
            width: childrenRect.width
            color: "#77a50026"
            ListView {
                visible: statusQueue.size
                width: contentItem.childrenRect.width
                height: 30 * statusQueue.size
                model: statusQueue
                delegate: Text {
                    padding: 5
                    height: 30
                    verticalAlignment: Text.AlignVCenter
                    text: display
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

        Component {
            id: interfaceComponent
            ComboBox {
                id: interfaceComboBox
                model: controller.interfaceModel
                textRole: "name"

                Binding {
                    target: controller.interfaceModel
                    property: "currentIndex"
                    value: interfaceComboBox.currentIndex
                }
            }
        }

        Component {
            id: interfaceWindowsComponent
            ComboBox {
                id: interfaceComboBox
                model: controller.windowsInterfaceModel
                textRole: "name"

                Binding {
                    target: controller.windowsInterfaceModel
                    property: "currentIndex"
                    value: interfaceComboBox.currentIndex
                }
            }
        }

        Component {
            id: scanProcessComponent

            Switch {
                id: connectedScan
                text: "connected or scan"
                checked: controller.scan

                Binding {
                    target: controller
                    property: "scan"
                    value: connectedScan.checked
                }
            }
        }

        Component {
            id: iperfComponent
            Row {
                Switch {
                    id: iperf
                    text: "iperf"
                    checked: controller.iperf

                    Binding {
                        target: controller
                        property: "iperf"
                        value: iperf.checked
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: controller.iperf
                    text: "server: "
                }

                TextInput {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: controller.iperf
                    id: serverHostInput
                    text: controller.iPerfScan.serverHost
                    Binding {
                        target: controller.iPerfScan
                        property: "serverHost"
                        value: serverHostInput.text
                    }
                }

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: controller.iperf
                    text: ":"
                }

                TextInput {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: controller.iperf
                    id: serverPortInput
                    text: controller.iPerfScan.serverPort
                    validator: IntValidator {
                        bottom: 0
                        top: 65536
                    }
                    Binding {
                        target: controller.iPerfScan
                        property: "serverPort"
                        value: serverPortInput.text
                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.topMargin: 5

            Loader {
                Layout.fillWidth: true
                Component.onCompleted: {
                    if (Qt.platform.os == "linux") {
                        this.sourceComponent = interfaceComponent
                    }
                }
            }

            Loader {
                Layout.fillWidth: true
                Component.onCompleted: {
                    if (Qt.platform.os == "windows") {
                        this.sourceComponent = interfaceWindowsComponent
                    }
                }
            }

            Loader {
                Layout.fillWidth: true
                Component.onCompleted: {
                    if (Qt.platform.os == "linux"
                            || Qt.platform.os == "android") {
                        this.sourceComponent = scanProcessComponent
                    }
                }
            }

            Loader {
                Layout.fillWidth: true
                Component.onCompleted: {
                    if (Qt.platform.os == "linux") {
                        this.sourceComponent = iperfComponent
                    }
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
