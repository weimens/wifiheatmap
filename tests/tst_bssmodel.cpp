#include <QObject>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include "bssmodel.h"

class BssModelTest : public QObject {
  Q_OBJECT

private slots:
  void add() {
    Measurements m;
    BssModel bssModel;
    bssModel.measurementsChanged(&m);

    QCOMPARE(bssModel.rowCount(), 0);

    // add two entries
    // ==============
    m.addPosition(Position{QPoint(42, 42)});
    m.newMeasurementsAtPosition(
        Position{QPoint(42, 42)},
        {
            {Bss{"36:2c:94:64:26:28", "chips", 2437, 6}, WiFiSignal, -83.0},
            {Bss{"08:96:d7:9d:cd:c2", "chookies", 2457, 10}, WiFiSignal, -58.0},
        });
    // ==============
    QCOMPARE(bssModel.rowCount(), 2);

    // add two other entries with same bss
    // ==============
    m.addPosition(Position{QPoint(3, 3)});
    m.newMeasurementsAtPosition(
        Position{QPoint(3, 3)},
        {
            {Bss{"36:2c:94:64:26:28", "chips", 2437, 6}, WiFiSignal, -58.0},
            {Bss{"08:96:d7:9d:cd:c2", "chookies", 2457, 10}, WiFiSignal, -83.0},
        });
    // ==============
    QCOMPARE(bssModel.rowCount(), 2);
  }
  void selection() {
    qRegisterMetaType<QVector<Bss>>("QVector<Bss>");

    Measurements m;
    BssModel bssModel;
    bssModel.measurementsChanged(&m);

    QSignalSpy selectedBssChanged(&bssModel, &BssModel::selectedBssChanged);

    // init with some entries.
    // ==============
    m.addPosition(Position{QPoint(42, 42)});
    m.newMeasurementsAtPosition(
        Position{QPoint(42, 42)},
        {
            {Bss{"36:2c:94:64:26:28", "chips", 2437, 6}, WiFiSignal, -83.0},
            {Bss{"08:96:d7:9d:cd:c2", "chookies", 2457, 10}, WiFiSignal, -58.0},
        });
    // ==============
    QTRY_COMPARE_WITH_TIMEOUT(selectedBssChanged.count(), 0, 200);

    // select first entry
    // ==============
    bssModel.setData(bssModel.index(0, 4), true, Qt::CheckStateRole);
    // ==============
    QTRY_COMPARE_WITH_TIMEOUT(selectedBssChanged.count(), 1, 200);
    auto arguments1 = selectedBssChanged.takeAt(0);
    QVector<Bss> expected_result1 = {
        Bss{"36:2c:94:64:26:28", "chips", 2437, 6}};
    QTRY_COMPARE_WITH_TIMEOUT(arguments1.at(0).value<QVector<Bss>>(),
                              expected_result1, 200);

    // unsselect first entry
    // ==============
    bssModel.setData(bssModel.index(0, 4), false, Qt::CheckStateRole);
    // ==============
    QTRY_COMPARE_WITH_TIMEOUT(selectedBssChanged.count(), 1, 200);
    auto arguments2 = selectedBssChanged.takeAt(0);
    QVector<Bss> expected_result2 = {};
    QTRY_COMPARE_WITH_TIMEOUT(arguments2.at(0).value<QVector<Bss>>(),
                              expected_result2, 200);
  }
};

QTEST_MAIN(BssModelTest)
#include "tst_bssmodel.moc"
