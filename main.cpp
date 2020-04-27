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

#ifdef Q_OS_ANDROID
#include "androidscan.h"
#elif defined(Q_OS_LINUX)
#include "interfacemodel.h"
#include "linuxscan.h"
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

  MeasurementModel *posModel = new MeasurementModel(&app);
  QObject::connect(document, &Document::measurementsChanged, posModel,
                   &MeasurementModel::measurementsChanged);
  posModel->measurementsChanged(document->measurements());

  HeatMapProvider *heatmap = new HeatMapProvider();
  engine.addImageProvider(QLatin1String("heatmap"), heatmap);
  ImageProvider *imageProvider = new ImageProvider(document);
  engine.addImageProvider(QLatin1String("document"), imageProvider);
  HeatMapCalc *heatMapCalc = new HeatMapCalc(heatmap, document, document);

  BssModel bssModel;
  QObject::connect(document, &Document::measurementsChanged, &bssModel,
                   &BssModel::measurementsChanged);
  bssModel.measurementsChanged(document->measurements());
  const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

#ifdef Q_OS_ANDROID
  AndroidScan *androidScan = new AndroidScan(&app);
  QObject::connect(androidScan, &AndroidScan::scanFinished, posModel,
                   &MeasurementModel::scanFinished);
  QObject::connect(androidScan, &AndroidScan::scanFailed, posModel,
                   &MeasurementModel::scanFailed);
  QObject::connect(androidScan, &AndroidScan::scanStarted, posModel,
                   &MeasurementModel::scanStarted);
  ctxt->setContextProperty("androidScan", androidScan);
#elif defined(Q_OS_LINUX)
  LinuxScan *linuxScan = new LinuxScan(&app);
  QObject::connect(linuxScan, &LinuxScan::scanFinished, posModel,
                   &MeasurementModel::scanFinished);
  QObject::connect(linuxScan, &LinuxScan::scanFailed, posModel,
                   &MeasurementModel::scanFailed);
  QObject::connect(linuxScan, &LinuxScan::scanStarted, posModel,
                   &MeasurementModel::scanStarted);
  InterfaceModel *interfaceModel = new InterfaceModel(&app);
  QObject::connect(interfaceModel, &InterfaceModel::currentInterfaceChanged,
                   linuxScan, &LinuxScan::setInterfaceIndex);
  ctxt->setContextProperty("linuxScan", linuxScan);
  ctxt->setContextProperty("interfaceModel", interfaceModel);
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
