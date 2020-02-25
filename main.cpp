#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSortFilterProxyModel>

#include "bssmodel.h"
#include "document.h"
#include "heatmapprovider.h"
#include "interfacemodel.h"
#include "measurementmodel.h"
#include "netlinkwrapper.h"

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QGuiApplication app(argc, argv);
  app.setApplicationName("wifi heat map");
  app.setOrganizationDomain("com.github.weimens.wifiheatmap");

  QQmlApplicationEngine engine;
  QQmlContext *ctxt = engine.rootContext();

  MeasurementModel *posModel = new MeasurementModel;
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

  ctxt->setContextProperty("posModel", posModel);
  ctxt->setContextProperty("interfaceModel", interfaceModel);
  ctxt->setContextProperty("document", document);

  QSortFilterProxyModel *proxyBssModel = new QSortFilterProxyModel();
  proxyBssModel->setSourceModel(&bssModel);
  proxyBssModel->setSortRole(bssItem::Roles::ssidRole);
  proxyBssModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  proxyBssModel->sort(0);
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
