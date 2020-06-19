#include <QApplication>
#include <QFileInfo>
#include <QFontDatabase>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSortFilterProxyModel>
#include <QUndoStack>

#include "bssmodel.h"
#include "document.h"
#include "heatmap.h"
#include "imageprovider.h"
#include "measurementcontroller.h"
#include "measurementtypemodel.h"
#include "qmlsortfilterproxymodel.h"
#include "statusqueue.h"

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QApplication app(argc, argv);
  app.setApplicationName("com.github.weimens.wifiheatmap");
  app.setWindowIcon(
      QIcon(":/installer/XDGData/com.github.weimens.wifiheatmap.svg"));

  QQmlApplicationEngine engine;
  QQmlContext *ctxt = engine.rootContext();

  auto undoStack = new QUndoStack(&app);

  Document *document = new Document(undoStack, &app);
  document->newDocument();

  MeasurementModel *posModel = new MeasurementModel(undoStack, &app);
  QObject::connect(document, &Document::measurementsChanged, posModel,
                   &MeasurementModel::measurementsChanged);
  posModel->measurementsChanged(document->measurements());

  MeasurementTypeModel *typeModel = new MeasurementTypeModel(&app);
  QObject::connect(document, &Document::measurementsChanged, typeModel,
                   &MeasurementTypeModel::measurementsChanged);
  typeModel->measurementsChanged(document->measurements());

  HeatMapLegend *heatMapLegend = new HeatMapLegend(&app);
  QObject::connect(typeModel, &MeasurementTypeModel::selectedTypeChanged,
                   heatMapLegend, &HeatMapLegend::selectedTypeChanged);

  HeatMapProvider *heatmap = new HeatMapProvider();
  engine.addImageProvider(QLatin1String("heatmap"), heatmap);
  ImageProvider *imageProvider = new ImageProvider(document);
  engine.addImageProvider(QLatin1String("document"), imageProvider);
  HeatMapCalc *heatMapCalc =
      new HeatMapCalc(heatmap, document, heatMapLegend, document);

  BssModel bssModel;
  QObject::connect(document, &Document::measurementsChanged, &bssModel,
                   &BssModel::measurementsChanged);
  bssModel.measurementsChanged(document->measurements());
  const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

  StatusQueue *statusQueue = new StatusQueue(&app);

  MeasurementController *controller =
      new MeasurementController(statusQueue, &app);
  QObject::connect(controller, &MeasurementController::scanFinished, posModel,
                   &MeasurementModel::scanFinished);
  QObject::connect(controller, &MeasurementController::scanFailed, posModel,
                   &MeasurementModel::scanFailed);
  QObject::connect(controller, &MeasurementController::scanStarted, posModel,
                   &MeasurementModel::scanStarted);

  ctxt->setContextProperty("posModel", posModel);
  ctxt->setContextProperty("controller", controller);
  ctxt->setContextProperty("document", document);
  ctxt->setContextProperty("fixedFont", fixedFont);
  ctxt->setContextProperty("heatMapCalc", heatMapCalc);
  ctxt->setContextProperty("undoStack", undoStack);
  ctxt->setContextProperty("typeModel", typeModel);
  ctxt->setContextProperty("heatMapLegend", heatMapLegend);
  ctxt->setContextProperty("typeModel", typeModel);
  ctxt->setContextProperty("statusQueue", statusQueue);

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
