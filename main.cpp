#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "measurementmodel.h"
#include "heatmapprovider.h"

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;

  MeasurementModel *posModel = new MeasurementModel;
  posModel->append({QPoint(155, 1127), -79.3});
  posModel->append({QPoint(-54, 1309), -79.3});
  posModel->append({QPoint(133, 918), -71});
  posModel->append({QPoint(264, 983), -75});
  posModel->append({QPoint(410, 1118), -63.7});
  posModel->append({QPoint(411, 955), -58.6});
  posModel->append({QPoint(321, 849), -68.3});
  posModel->append({QPoint(557, 1128), -66.1});
  posModel->append({QPoint(177, 690), -58.2});
  posModel->append({QPoint(290, 1236), -43.4});
  posModel->append({QPoint(549, 829), -70});
  posModel->append({QPoint(658, 977), -66.1});
  posModel->append({QPoint(212, 514), -51.2});
  posModel->append({QPoint(135, 481), -65.3});
  posModel->append({QPoint(935, 1290), -68.1});
  posModel->append({QPoint(935, -44), -46.3});
  posModel->append({QPoint(760, 818), -64.7});
  posModel->append({QPoint(405, 429), -72.5});
  posModel->append({QPoint(-40, -31), -57.4});
  posModel->append({QPoint(284, 242), -66.9});

  HeatMapProvider *heatmap = new HeatMapProvider(posModel);
  engine.addImageProvider(QLatin1String("heatmap"), heatmap);
  QQmlContext *ctxt = engine.rootContext();
  ctxt->setContextProperty("posModel", posModel);

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
