#include <QObject>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include "document.h"

class DocumentTest : public QObject {
  Q_OBJECT

private slots:

  void changeMapImage() {
    Document document;
    document.newDocument();

    QSignalSpy mapImageChanged(&document, &Document::mapImageChanged);

    document.setMapImageUrl(QUrl(":/A4_120dpi.png"));

    QTRY_COMPARE_WITH_TIMEOUT(mapImageChanged.count(), 1, 200);
  }

  void newDocument() {
    Document document;
    QSignalSpy mapImageChanged(&document, &Document::mapImageChanged);
    QSignalSpy measurementsChanged(&document, &Document::measurementsChanged);

    document.newDocument();

    QTRY_COMPARE_WITH_TIMEOUT(mapImageChanged.count(), 1, 200);
    QTRY_COMPARE_WITH_TIMEOUT(measurementsChanged.count(), 1, 200);
  }

  void needsSaving() {
    Document document;
    document.newDocument();
    QSignalSpy needsSavingChanged(&document, &Document::needsSavingChanged);

    QCOMPARE(document.property("needsSaving").toBool(), false);

    // add entries
    document.measurements()->appendItem(
        {QPoint(42, 42),
         {
             {"36:2c:94:64:26:28",
              ScanInfo{"36:2c:94:64:26:28", "chips", 0, 2437, -83.0, 6}},
             {"08:96:d7:9d:cd:c2",
              ScanInfo{"08:96:d7:9d:cd:c2", "chookies", 0, 2457, -58.0, 10}},
         }});

    QCOMPARE(document.property("needsSaving").toBool(), true);
    QTRY_COMPARE_WITH_TIMEOUT(needsSavingChanged.count(), 1, 200);
  }
};

QTEST_MAIN(DocumentTest)

#include "tst_document.moc"
