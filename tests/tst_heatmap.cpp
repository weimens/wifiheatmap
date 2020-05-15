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
    QUndoStack undostack;
    Document document(&undostack);
    document.newDocument();

    HeatMapLegend heatMapLegend;
    HeatMapProvider heatmap = HeatMapProvider();
    HeatMapCalc *heatMapCalc =
        new HeatMapCalc(&heatmap, &document, &heatMapLegend, &document);

    QSignalSpy heatMapReady(heatMapCalc, &HeatMapCalc::heatMapReady);

    // add entries
    document.measurements()->addPosition(Position{QPoint(42, 42)});
    document.measurements()->newMeasurementsAtPosition(
        Position{QPoint(42, 42)},
        {
            {Bss{"36:2c:94:64:26:28", "chips", 2437, 6}, -83.0},
            {Bss{"08:96:d7:9d:cd:c2", "chookies", 2457, 10}, -58.0},
        });
    document.measurements()->addPosition(Position{QPoint(420, 210)});
    document.measurements()->newMeasurementsAtPosition(
        Position{QPoint(420, 210)},
        {
            {Bss{"36:2c:94:64:26:28", "chips", 2437, 6}, -83.0},
            {Bss{"08:96:d7:9d:cd:c2", "chookies", 2457, 10}, -58.0},
        });
    document.measurements()->addPosition(Position{QPoint(420, 210)});
    document.measurements()->newMeasurementsAtPosition(
        Position{QPoint(300, 300)},
        {
            {Bss{"36:2c:94:64:26:28", "chips", 2437, 6}, -83.0},
            {Bss{"08:96:d7:9d:cd:c2", "chookies", 2457, 10}, -58.0},
        });

    QTRY_COMPARE_WITH_TIMEOUT(heatMapReady.count(), 2, 200);

    document.measurements()->selectedBssChanged(
        QVector<Bss>() << Bss{"36:2c:94:64:26:28", "chips", 2437, 6});

    QTRY_COMPARE_WITH_TIMEOUT(heatMapReady.count(), 3, 200);

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
