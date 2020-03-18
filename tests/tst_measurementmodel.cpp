#include <QObject>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include "measurementmodel.h"

class MeasurementModelTest : public QObject {
  Q_OBJECT

private slots:
  void startFinished() {
    Measurements m;
    MeasurementModel posModel;
    posModel.measurementsChanged(&m);

    QSignalSpy heatMapChanged(&m, &Measurements::heatMapChanged);
    QSignalSpy ItemChanged(&m, &Measurements::ItemChanged);
    QSignalSpy dataChanged(&posModel, &MeasurementModel::dataChanged);

    // startScan
    // ==============
    posModel.scanStarted(QPoint(3, 3));
    // ==============
    QCOMPARE(posModel.rowCount(), 1);
    QCOMPARE(posModel.data(posModel.index(posModel.rowCount() - 1),
                           MeasurementModel::Roles::stateRole),
             false);
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 0, 200);
    QTRY_COMPARE_WITH_TIMEOUT(ItemChanged.count(), 0, 200);
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 0, 200);

    // scanFinished
    // ==============
    posModel.scanFinished(QList<ScanInfo>() << ScanInfo{
                              "36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6});
    // ==============
    QCOMPARE(posModel.rowCount(), 1);
    QCOMPARE(posModel.data(posModel.index(posModel.rowCount() - 1),
                           MeasurementModel::Roles::stateRole),
             true);
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(ItemChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 2, 200);
  }

  void startFaild() {
    Measurements m;
    MeasurementModel posModel;
    posModel.measurementsChanged(&m);

    QSignalSpy heatMapChanged(&m, &Measurements::heatMapChanged);
    QSignalSpy ItemChanged(&m, &Measurements::ItemChanged);
    QSignalSpy postItemRemoved(&m, &Measurements::postItemRemoved);

    // startScan
    // ==============
    posModel.scanStarted(QPoint(42, 42));
    // ==============
    QCOMPARE(posModel.rowCount(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 0, 200);
    QTRY_COMPARE_WITH_TIMEOUT(ItemChanged.count(), 0, 200);

    // scanFailed
    // ==============
    posModel.scanFailed(1);
    // ==============
    QCOMPARE(posModel.rowCount(), 0);
    // QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 0, 200);
    QTRY_COMPARE_WITH_TIMEOUT(ItemChanged.count(), 0, 200);
    QTRY_COMPARE_WITH_TIMEOUT(postItemRemoved.count(), 1, 200);
  }

  void updatePos() {
    Measurements m;
    MeasurementModel posModel;
    posModel.measurementsChanged(&m);

    QSignalSpy heatMapChanged(&m, &Measurements::heatMapChanged);
    QSignalSpy ItemChanged(&m, &Measurements::ItemChanged);
    QSignalSpy dataChanged(&posModel, &MeasurementModel::dataChanged);

    m.appendItem(
        {QPoint(42, 42),
         {
             {"36:2c:94:64:26:28",
              ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6}},
             {"08:96:d7:9d:cd:c2",
              ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -58.0, 10}},
         }});
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 0, 200);
    QTRY_COMPARE_WITH_TIMEOUT(ItemChanged.count(), 0, 200);
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 0, 200);

    // update pos
    // ==============
    posModel.setData(posModel.index(0), QPoint(21, 21),
                     MeasurementModel::Roles::posRole);
    // ==============
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(ItemChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 2, 200);
  }

  void BssSelection() {
    Measurements m;
    MeasurementModel posModel;
    posModel.measurementsChanged(&m);

    QSignalSpy dataChanged(&posModel, &MeasurementModel::dataChanged);

    // init with some entries. no bss selected
    // ==============
    MeasurementItem item;
    item.pos = QPoint(42, 42);
    item.scan["36:2c:94:64:26:28"] =
        ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6};
    item.scan["08:96:d7:9d:cd:c2"] =
        ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -58.0, 10};
    m.appendItem(item);
    MeasurementItem item1;
    item.pos = QPoint(42, 42);
    item.scan["36:2c:94:64:26:28"] =
        ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -58.0, 6};
    item.scan["08:96:d7:9d:cd:c2"] =
        ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -83.0, 10};
    m.appendItem(item1);
    // ==============

    // select one bss
    // ==============
    m.bssChanged(QList<QString>() << "08:96:d7:9d:cd:c2");
    // ==============
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 2, 200);
  }
};

QTEST_MAIN(MeasurementModelTest)

#include "tst_measurementmodel.moc"
