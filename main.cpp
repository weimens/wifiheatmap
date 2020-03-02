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
#include "interfacemodel.h"
#include "measurementmodel.h"
#include "netlinkwrapper.h"
#include "qmlsortfilterproxymodel.h"
#include "trigger_scan.h"

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication app(argc, argv);
  app.setApplicationName("wifi heat map");
  app.setOrganizationDomain("com.github.weimens.wifiheatmap");

  QQmlApplicationEngine engine;
  QQmlContext *ctxt = engine.rootContext();

  Document *document = new Document(&app);
  document->newDocument();
  TriggerScan *triggerScan = new TriggerScan(&app);
  MeasurementModel *posModel = new MeasurementModel(document, triggerScan, &app);
  HeatMapProvider *heatmap = new HeatMapProvider();
  engine.addImageProvider(QLatin1String("heatmap"), heatmap);
  ImageProvider *imageProvider = new ImageProvider(document);
  engine.addImageProvider(QLatin1String("document"), imageProvider);
  HeatMapCalc *heatMapCalc = new HeatMapCalc(heatmap, document, document);

  InterfaceModel *interfaceModel = new InterfaceModel(&app);
  QObject::connect(interfaceModel, &InterfaceModel::currentInterfaceChanged,
                   posModel, &MeasurementModel::setInterfaceIndex);

  NetLink::Nl80211 nl80211;
  NetLink::MessageInterface msg;
  nl80211.sendMessageWait(&msg);
  for (auto interface : msg.getInterfaces()) {
    interfaceModel->append(interface.first, interface.second.c_str());
  }

  BssModel bssModel(document);
  const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

  ctxt->setContextProperty("triggerScan", triggerScan);
  ctxt->setContextProperty("posModel", posModel);
  ctxt->setContextProperty("interfaceModel", interfaceModel);
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
