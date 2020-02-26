#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSortFilterProxyModel>
#include <QFontDatabase>

#include "bssmodel.h"
#include "document.h"
#include "heatmapprovider.h"
#include "interfacemodel.h"
#include "measurementmodel.h"
#include "netlinkwrapper.h"
#include "trigger_scan.h"
#include "qmlsortfilterproxymodel.h"

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication app(argc, argv);
  app.setApplicationName("wifi heat map");
  app.setOrganizationDomain("com.github.weimens.wifiheatmap");

  QQmlApplicationEngine engine;
  QQmlContext *ctxt = engine.rootContext();

  TriggerScan *triggerScan = new TriggerScan(&app);
  MeasurementModel *posModel = new MeasurementModel(triggerScan, &app);
  HeatMapProvider *heatmap = new HeatMapProvider(posModel);
  engine.addImageProvider(QLatin1String("heatmap"), heatmap);
  Document *document = new Document(posModel);

  InterfaceModel *interfaceModel = new InterfaceModel(&app);
  QObject::connect(interfaceModel, &InterfaceModel::currentInterfaceChanged,
                   posModel, &MeasurementModel::setInterfaceIndex);

  NetLink::Nl80211 nl80211;
  NetLink::MessageInterface msg;
  nl80211.sendMessageWait(&msg);
  for (auto interface : msg.getInterfaces()) {
    interfaceModel->append(interface.first, interface.second.c_str());
  }

  BssModel bssModel;
  QObject::connect(&bssModel, &BssModel::selectedBssChanged, posModel,
                   &MeasurementModel::bssChanged);
  QObject::connect(posModel, &MeasurementModel::bssAdded, &bssModel,
                   &BssModel::addBss);

  const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

  ctxt->setContextProperty("triggerScan", triggerScan);
  ctxt->setContextProperty("posModel", posModel);
  ctxt->setContextProperty("interfaceModel", interfaceModel);
  ctxt->setContextProperty("document", document);
  ctxt->setContextProperty("fixedFont", fixedFont);

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

  return app.exec();
}
