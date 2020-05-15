#include <QObject>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include "measurements.h"

class MeasurementsTest : public QObject {
  Q_OBJECT

private slots:
  void createUpdateDelete() {
    Measurements m;
    QSignalSpy prePositionAppended(&m, &Measurements::prePositionAppended);
    QSignalSpy postPositionAppended(&m, &Measurements::postPositionAppended);
    QSignalSpy heatMapChanged(&m, &Measurements::heatMapChanged);
    QSignalSpy positionChanged(&m, &Measurements::positionChanged);

    // precondition
    QCOMPARE(m.positions().size(), 0);

    // append entry
    // ==============
    m.addPosition(Position{QPoint(3, 3)});
    // ==============
    QCOMPARE(m.positions().size(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(prePositionAppended.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(postPositionAppended.count(), 1, 200);
    // QTRY_COMPARE_WITH_TIMEOUT(bssAdded.count(), 0, 200);
    QTRY_COMPARE_WITH_TIMEOUT(positionChanged.count(), 0, 200);
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 0, 200);

    // update entry
    // ==============
    m.newMeasurementsAtPosition(
        Position{QPoint(3, 3)},
        {
            {Bss{"36:2c:94:64:26:28", "chips", 2437, 6}, WiFiSignal, -83.0},
        });
    // ==============
    QCOMPARE(m.positions().size(), 1);
    QCOMPARE(m.bsss().size(), 1);
    QCOMPARE(m.measurements().size(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(prePositionAppended.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(postPositionAppended.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(positionChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 1, 200);

    // update entry with same data //FIXME: more tests
    // ==============
    // item.scan["36:2c:94:64:26:28"] =
    //    ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6};
    // QCOMPARE(m.setItemAt(idx, item), false);
    // ==============

    QSignalSpy preMeasurementRemoved(&m, &Measurements::preMeasurementRemoved);
    QSignalSpy postMeasurementRemoved(&m,
                                      &Measurements::postMeasurementRemoved);

    // remove entry
    // ==============
    m.removePosition(Position{QPoint(3, 3)});
    // ==============
    QCOMPARE(m.positions().size(), 0);
    QCOMPARE(m.bsss().size(), 0);
    QCOMPARE(m.measurements().size(), 0);
    QTRY_COMPARE_WITH_TIMEOUT(prePositionAppended.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(postPositionAppended.count(), 1, 200);
    // QTRY_COMPARE_WITH_TIMEOUT(bssAdded.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(positionChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 2, 200);
    QTRY_COMPARE_WITH_TIMEOUT(preMeasurementRemoved.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(postMeasurementRemoved.count(), 1, 200);

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
    // QTRY_COMPARE_WITH_TIMEOUT(bssAdded.count(), 2, 200);
    QCOMPARE(m.positions().size(), 1);
    QCOMPARE(m.bsss().size(), 2);
    QCOMPARE(m.measurements().size(), 2);

    // add entry with already existing bss
    // ==============
    m.addPosition(Position{QPoint(3, 3)});
    m.newMeasurementsAtPosition(
        Position{QPoint(3, 3)},
        {
            {Bss{"36:2c:94:64:26:28", "chips", 2437, 6}, WiFiSignal, -83.0},
        });
    // ==============
    // QTRY_COMPARE_WITH_TIMEOUT(bssAdded.count(), 2, 200);
    QCOMPARE(m.positions().size(), 2);
    QCOMPARE(m.bsss().size(), 2);
    QCOMPARE(m.measurements().size(), 3);
  }

  void BssSelection() {
    Measurements m;
    QSignalSpy heatMapChanged(&m, &Measurements::heatMapChanged);

    // precondition
    QCOMPARE(m.maxZAt(0), NAN);
    QCOMPARE(m.positions().size(), 0);

    // init with some entries. no bss selected
    // ==============
    m.addPosition(Position{QPoint(42, 42)});
    m.newMeasurementsAtPosition(
        Position{QPoint(42, 42)},
        {
            {Bss{"36:2c:94:64:26:28", "chips", 2437, 6}, WiFiSignal, -83.0},
            {Bss{"08:96:d7:9d:cd:c2", "chookies", 2457, 10}, WiFiSignal, -58.0},
        });
    // ==============
    QCOMPARE(m.maxZAt(0), NAN);
    QCOMPARE(m.positions().at(0).pos, QPoint(42, 42));
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 1, 200);

    // select one bss
    // ==============
    m.selectedBssChanged(QVector<Bss>()
                         << Bss{"08:96:d7:9d:cd:c2", "chookies", 2457, 10});
    // ==============
    QCOMPARE(m.maxZAt(0), -58.0);
    QCOMPARE(m.positions().at(0).pos, QPoint(42, 42));
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 2, 200);

    // remove selection
    // ==============
    m.selectedBssChanged({});
    // ==============
    QCOMPARE(m.maxZAt(0), NAN);
    QCOMPARE(m.positions().at(0).pos, QPoint(42, 42));
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 3, 200);

    // select other bss
    // ==============
    m.selectedBssChanged(QVector<Bss>()
                         << Bss{"36:2c:94:64:26:28", "chips", 2437, 6});
    // ==============
    QCOMPARE(m.maxZAt(0), -83.0);
    QCOMPARE(m.positions().at(0).pos, QPoint(42, 42));
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 4, 200);

    // select tow bss
    // ==============
    m.selectedBssChanged(QVector<Bss>()
                         << Bss{"08:96:d7:9d:cd:c2", "chookies", 2457, 10}
                         << Bss{"36:2c:94:64:26:28", "chips", 2437, 6});
    // ==============
    QCOMPARE(m.maxZAt(0), -58.0);
    QCOMPARE(m.positions().at(0).pos, QPoint(42, 42));
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 5, 200);
  }
};

QTEST_MAIN(MeasurementsTest)

#include "tst_measurements.moc"
