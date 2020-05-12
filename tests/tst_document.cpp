#include <QObject>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include "document.h"

class DocumentTest : public QObject {
  Q_OBJECT

private slots:

  void changeMapImage() {
    QUndoStack undostack;
    Document document(&undostack);
    document.newDocument();

    QSignalSpy mapImageChanged(&document, &Document::mapImageChanged);

    document.setMapImageUrl(QUrl(":/A4_120dpi.png"));

    QTRY_COMPARE_WITH_TIMEOUT(mapImageChanged.count(), 1, 200);
  }

  void newDocument() {
    QUndoStack undostack;
    Document document(&undostack);
    QSignalSpy mapImageChanged(&document, &Document::mapImageChanged);
    QSignalSpy measurementsChanged(&document, &Document::measurementsChanged);

    document.newDocument();

    QTRY_COMPARE_WITH_TIMEOUT(mapImageChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(measurementsChanged.count(), 1, 200);
  }

  void needsSaving() {
    QUndoStack undostack;
    Document document(&undostack);
    document.newDocument();
    QSignalSpy needsSavingChanged(&document, &Document::needsSavingChanged);

    QCOMPARE(document.property("needsSaving").toBool(), false);

    // add entries
    document.measurements()->addPosition(Position{QPoint(42, 42)});
    document.measurements()->newMeasurementsAtPosition(
        Position{QPoint(42, 42)},
        {
            {Bss{"36:2c:94:64:26:28", "chips", 2437, 6}, -83.0},
            {Bss{"08:96:d7:9d:cd:c2", "chookies", 2457, 10}, -58.0},
        });

    QCOMPARE(document.property("needsSaving").toBool(), true);
    QTRY_COMPARE_WITH_TIMEOUT(needsSavingChanged.count(), 1, 200);
  }
};

QTEST_MAIN(DocumentTest)

#include "tst_document.moc"
