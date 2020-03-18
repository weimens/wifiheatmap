#include <QObject>
#include <QSignalSpy>
#include <QtTest/QtTest>
#include <cmath>

#include "heatmap.h"

class HeatMapTest : public QObject {
  Q_OBJECT

private slots:
  void legend() {
    HeatMapProvider heatmap = HeatMapProvider();
    auto size = QSize();
    auto image = heatmap.requestImage("legend", &size, QSize());
    QCOMPARE(image.size() == size, true);
  }

  void heatMapCalc() {
    Document document;
    document.newDocument();

    HeatMapProvider heatmap = HeatMapProvider();
    HeatMapCalc *heatMapCalc =
        new HeatMapCalc(&heatmap, &document, &document); // FIXME no pointer

    QSignalSpy heatMapReady(heatMapCalc, &HeatMapCalc::heatMapReady);

    // add entries
    document.measurements()->appendItem(
        {QPoint(42, 42),
         {
             {"36:2c:94:64:26:28",
              ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6}},
             {"08:96:d7:9d:cd:c2",
              ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -58.0, 10}},
         }});
    document.measurements()->appendItem(
        {QPoint(420, 210),
         {
             {"36:2c:94:64:26:28",
              ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6}},
             {"08:96:d7:9d:cd:c2",
              ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -58.0, 10}},
         }});
    document.measurements()->appendItem(
        {QPoint(300, 300),
         {
             {"36:2c:94:64:26:28",
              ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6}},
             {"08:96:d7:9d:cd:c2",
              ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -58.0, 10}},
         }});

    QTRY_COMPARE_WITH_TIMEOUT(heatMapReady.count(), 0, 200);

    document.measurements()->bssChanged(QList<QString>()
                                        << "36:2c:94:64:26:28");

    QTRY_COMPARE_WITH_TIMEOUT(heatMapReady.count(), 1, 200);

    auto size = QSize();
    auto image = heatmap.requestImage("heatmap", &size, QSize());
    QCOMPARE(image.size() == size, true);

    QSize size_orig = document.mapImage().size();
    qreal ratio_orig =
        size_orig.width() / static_cast<qreal>(size_orig.height());
    qreal ratio =
        image.size().width() / static_cast<qreal>(image.size().height());
    QCOMPARE(std::abs(ratio_orig - ratio) < 0.1, true);
  }
};

QTEST_MAIN(HeatMapTest)

#include "tst_heatmap.moc"
