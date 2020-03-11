#include <QApplication>
#include <QFileInfo>
#include <QFontDatabase>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSortFilterProxyModel>

#include "bssmodel.h"
#include "document.h"
#include "heatmap.h"
#include "imageprovider.h"
#include "measurementmodel.h"
#include "qmlsortfilterproxymodel.h"

#ifndef Q_OS_ANDROID
#include "interfacemodel.h"
#include "trigger_scan.h"
#endif

#ifdef Q_OS_ANDROID
#include "androidscan.h"
#endif

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication app(argc, argv);
  app.setApplicationName("wifi heat map");
  app.setOrganizationDomain("com.github.weimens.wifiheatmap");

  QQmlApplicationEngine engine;
  QQmlContext *ctxt = engine.rootContext();

  Document *document = new Document(&app);
  document->newDocument();

  MeasurementModel *posModel = new MeasurementModel(document, &app);
  HeatMapProvider *heatmap = new HeatMapProvider();
  engine.addImageProvider(QLatin1String("heatmap"), heatmap);
  ImageProvider *imageProvider = new ImageProvider(document);
  engine.addImageProvider(QLatin1String("document"), imageProvider);
  HeatMapCalc *heatMapCalc = new HeatMapCalc(heatmap, document, document);

  BssModel bssModel(document);
  const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

#ifndef Q_OS_ANDROID
  TriggerScan *triggerScan = new TriggerScan(posModel, &app);
  InterfaceModel *interfaceModel = new InterfaceModel(&app);
  QObject::connect(interfaceModel, &InterfaceModel::currentInterfaceChanged,
                   triggerScan, &TriggerScan::setInterfaceIndex);
  ctxt->setContextProperty("triggerScan", triggerScan);
  ctxt->setContextProperty("interfaceModel", interfaceModel);
#endif

#ifdef Q_OS_ANDROID
  AndroidScan *androidScan = new AndroidScan(posModel, &app);
  ctxt->setContextProperty("androidScan", androidScan);
#endif

  ctxt->setContextProperty("posModel", posModel);
  ctxt->setContextProperty("document", document);
  ctxt->setContextProperty("fixedFont", fixedFont);
  ctxt->setContextProperty("heatMapCalc", heatMapCalc);

  QmlSortFilterProxyModel *proxyBssModel = new QmlSortFilterProxyModel(&app);
  proxyBssModel->setSourceModel(&bssModel);
  proxyBssModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  ctxt->setContextProperty("bssmodel", proxyBssModel);

  const QUrl url(QStringLiteral("qrc:/main.qml"));
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
          QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);
  engine.load(url);

  if (argc == 2) {
    QString path(argv[1]);
    QFileInfo check_file(path);
    if (check_file.exists() && check_file.isFile()) {
      document->load(QUrl::fromLocalFile(path));
    }
  }

  return app.exec();
}
