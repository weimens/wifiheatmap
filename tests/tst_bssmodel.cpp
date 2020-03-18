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
    m.appendItem(
        {QPoint(42, 42),
         {
             {"36:2c:94:64:26:28",
              ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6}},
             {"08:96:d7:9d:cd:c2",
              ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -58.0, 10}},
         }});
    // ==============
    QCOMPARE(bssModel.rowCount(), 2);

    // add two other entries with same bss
    // ==============
    m.appendItem(
        {QPoint(3, 3),
         {
             {"36:2c:94:64:26:28",
              ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -58.0, 6}},
             {"08:96:d7:9d:cd:c2",
              ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -83.0, 10}},
         }});
    // ==============
    QCOMPARE(bssModel.rowCount(), 2);
  }
  void selection() {
    Measurements m;
    BssModel bssModel;
    bssModel.measurementsChanged(&m);

    QSignalSpy selectedBssChanged(&bssModel, &BssModel::selectedBssChanged);

    // init with some entries.
    // ==============
    m.appendItem(
        {QPoint(42, 42),
         {
             {"36:2c:94:64:26:28",
              ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6}},
             {"08:96:d7:9d:cd:c2",
              ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -58.0, 10}},
         }});
    // ==============
    QTRY_COMPARE_WITH_TIMEOUT(selectedBssChanged.count(), 0, 200);

    // select first entry
    // ==============
    bssModel.setData(bssModel.index(0, 0), true, Qt::CheckStateRole);
    // ==============
    QTRY_COMPARE_WITH_TIMEOUT(selectedBssChanged.count(), 1, 200);
  }
};

QTEST_MAIN(BssModelTest)
#include "tst_bssmodel.moc"
