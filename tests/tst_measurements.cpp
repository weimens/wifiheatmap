#include <QObject>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include "measurements.h"

class MeasurementsTest : public QObject {
  Q_OBJECT

private slots:
  void createUpdateDelete() {
    Measurements m;
    QSignalSpy preItemAppended(&m, &Measurements::preItemAppended);
    QSignalSpy postItemAppended(&m, &Measurements::postItemAppended);
    QSignalSpy bssAdded(&m, &Measurements::bssAdded);
    QSignalSpy heatMapChanged(&m, &Measurements::heatMapChanged);
    QSignalSpy itemChanged(&m, &Measurements::ItemChanged);

    // precondition
    QCOMPARE(m.items().size(), 0);

    // append entry
    // ==============
    m.appendItem({QPoint(3, 3), {}});
    // ==============
    QCOMPARE(m.items().size(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(preItemAppended.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(postItemAppended.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(bssAdded.count(), 0, 200);
    QTRY_COMPARE_WITH_TIMEOUT(itemChanged.count(), 0, 200);
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 0, 200);

    // update entry
    // ==============
    int idx = 0;
    MeasurementItem item = m.items().at(idx);
    item.scan["36:2c:94:64:26:28"] =
        ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6};
    QCOMPARE(m.setItemAt(idx, item), true);
    // ==============
    QCOMPARE(m.items().size(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(preItemAppended.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(postItemAppended.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(bssAdded.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(itemChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 1, 200);

    // update entry with same data //FIXME: more tests
    // ==============
    item.scan["36:2c:94:64:26:28"] =
        ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6};
    QCOMPARE(m.setItemAt(idx, item), false);
    // ==============

    QSignalSpy preItemRemoved(&m, &Measurements::preItemRemoved);
    QSignalSpy postItemRemoved(&m, &Measurements::postItemRemoved);

    // remove entry
    // ==============
    m.removeAt(idx);
    // ==============
    QCOMPARE(m.items().size(), 0);
    QTRY_COMPARE_WITH_TIMEOUT(preItemAppended.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(postItemAppended.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(bssAdded.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(itemChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 2, 200);
    QTRY_COMPARE_WITH_TIMEOUT(preItemRemoved.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(postItemRemoved.count(), 1, 200);

    // add two entries with one already existing bss
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
    QTRY_COMPARE_WITH_TIMEOUT(bssAdded.count(), 2, 200);

    // add entry with already existing bss
    // ==============
    m.appendItem(
        {QPoint(3, 3),
         {
             {"36:2c:94:64:26:28",
              ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6}},
         }});
    // ==============
    QTRY_COMPARE_WITH_TIMEOUT(bssAdded.count(), 2, 200);
  }

  void BssSelection() {
    Measurements m;
    QSignalSpy heatMapChanged(&m, &Measurements::heatMapChanged);

    // precondition
    QCOMPARE(m.maxZAt(0), NAN);
    QCOMPARE(m.posAt(0), QPoint());

    // init with some entries. no bss selected
    // ==============
    MeasurementItem item;
    item.pos = QPoint(42, 42);
    item.scan["36:2c:94:64:26:28"] =
        ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6};
    item.scan["08:96:d7:9d:cd:c2"] =
        ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -58.0, 10};
    m.appendItem(item);
    // ==============
    QCOMPARE(m.maxZAt(0), NAN);
    QCOMPARE(m.posAt(0), QPoint(42, 42));

    // select one bss
    // ==============
    m.bssChanged(QList<QString>() << "08:96:d7:9d:cd:c2");
    // ==============
    QCOMPARE(m.maxZAt(0), -58.0);
    QCOMPARE(m.posAt(0), QPoint(42, 42));
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 1, 200);

    // remove selection
    // ==============
    m.bssChanged({});
    // ==============
    QCOMPARE(m.maxZAt(0), NAN);
    QCOMPARE(m.posAt(0), QPoint(42, 42));
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 2, 200);

    // select other bss
    // ==============
    m.bssChanged(QList<QString>() << "36:2c:94:64:26:28");
    // ==============
    QCOMPARE(m.maxZAt(0), -83.0);
    QCOMPARE(m.posAt(0), QPoint(42, 42));
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 3, 200);

    // select tow bss
    // ==============
    m.bssChanged(QList<QString>() << "08:96:d7:9d:cd:c2"
                                  << "36:2c:94:64:26:28");
    // ==============
    QCOMPARE(m.maxZAt(0), -58.0);
    QCOMPARE(m.posAt(0), QPoint(42, 42));
    QTRY_COMPARE_WITH_TIMEOUT(heatMapChanged.count(), 4, 200);
  }
};

QTEST_MAIN(MeasurementsTest)

#include "tst_measurements.moc"
