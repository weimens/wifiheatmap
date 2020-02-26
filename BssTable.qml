import QtQuick 2.12
import QtQuick.Layouts 1.3
import Qt.labs.qmlmodels 1.0
import QtQuick.Controls 2.3

TableView {
    id: tableView
    model: bssmodel
    property var columnWidths: [200, 165, 45, 30, 30]

    columnWidthProvider: function (column) {
        return columnWidths[column]
    }
    rowHeightProvider: function (column) {
        return 30
    }

    topMargin: header.height
    Row {
        id: header
        height: 30
        y: tableView.contentY
        z: 2
        Repeater {
            model: tableView.columns > 0 ? tableView.columns : 1
            Rectangle {
                width: tableView.columnWidthProvider(modelData)
                height: parent.height
                Text {
                    anchors.fill: parent
                    text: bssmodel.headerData(modelData, Qt.Horizontal)
                    verticalAlignment: Text.AlignVCenter
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        bssmodel.sort(modelData)
                    }
                }
            }
        }
    }
    delegate: DelegateChooser {
        DelegateChoice {
            column: 4
            delegate: Rectangle {
                CheckBox {
                    anchors.fill: parent
                    checked: checkstate == Qt.Checked ? true : false
                    onCheckStateChanged: {
                        checkstate = checked ? Qt.Checked : Qt.Unchecked
                    }
                }
            }
        }
        DelegateChoice {
            column: 0
            delegate: Rectangle {
                Text {
                    text: display
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    anchors.fill: parent
                }
            }
        }
        DelegateChoice {
            delegate: Rectangle {
                Text {
                    font: fixedFont
                    text: display
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    anchors.fill: parent
                }
            }
        }
    }
}
