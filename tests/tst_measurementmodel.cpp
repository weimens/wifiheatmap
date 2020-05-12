#include <QObject>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include "measurementmodel.h"

class MeasurementModelTest : public QObject {
  Q_OBJECT

private slots:
  void startFinished() {
    Measurements m;
    QUndoStack undostack;
    MeasurementModel posModel(&undostack);
    posModel.measurementsChanged(&m);

    QSignalSpy heatMapChanged(&m, &Measurements::heatMapChanged);
    QSignalSpy positionChanged(&m, &Measurements::positionChanged);
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
    QTRY_COMPARE_WITH_TIMEOUT(positionChanged.count(), 0, 200);
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
    QTRY_COMPARE_WITH_TIMEOUT(positionChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 3, 200);
  }

  void startFaild() {
    Measurements m;
    QUndoStack undostack;
    MeasurementModel posModel(&undostack);
    posModel.measurementsChanged(&m);

    QSignalSpy heatMapChanged(&m, &Measurements::heatMapChanged);
    QSignalSpy ItemChanged(&m, &Measurements::positionChanged);
    QSignalSpy postPositionRemoved(&m, &Measurements::postPositionRemoved);

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
    QTRY_COMPARE_WITH_TIMEOUT(postPositionRemoved.count(), 1, 200);
  }

  void updatePos() {
    Measurements m;
    QUndoStack undostack;
    MeasurementModel posModel(&undostack);
    posModel.measurementsChanged(&m);

    QSignalSpy heatMapChanged(&m, &Measurements::heatMapChanged);
    QSignalSpy positionChanged(&m, &Measurements::positionChanged);
    QSignalSpy dataChanged(&posModel, &MeasurementModel::dataChanged);

    posModel.scanStarted(QPoint(42, 42));
    posModel.scanFinished({
        ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6},
        ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -58.0, 10},
    });
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(positionChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 3, 200);

    // update pos
    // ==============
    posModel.setData(posModel.index(0), QPoint(21, 21),
                     MeasurementModel::Roles::posRole);
    // ==============
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 2, 200);
    QTRY_COMPARE_WITH_TIMEOUT(positionChanged.count(), 2, 200);
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 6, 200);
  }

  void BssSelection() {
    Measurements m;
    QUndoStack undostack;
    MeasurementModel posModel(&undostack);
    posModel.measurementsChanged(&m);

    QSignalSpy dataChanged(&posModel, &MeasurementModel::dataChanged);

    // init with some entries. no bss selected
    // ==============
    posModel.scanStarted(QPoint(42, 42));
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 0, 200);
    posModel.scanFinished({
        ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6},
        ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -58.0, 10},
    });
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 3, 200);
    posModel.scanStarted(QPoint(3, 3));
    posModel.scanFinished({
        ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -58.0, 6},
        ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -83.0, 10},
    });
    // ==============
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 7, 200);

    // select one bss
    // ==============
    m.selectedBssChanged(QVector<Bss>()
                         << Bss{"08:96:d7:9d:cd:c2", "chookies", 2457, 10});
    // ==============
    QTRY_COMPARE_WITH_TIMEOUT(dataChanged.count(), 9, 200);
  }
};

QTEST_MAIN(MeasurementModelTest)

#include "tst_measurementmodel.moc"
