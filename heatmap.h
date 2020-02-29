#pragma once

#include <QImage>
#include <QQuickImageProvider>

#include "measurementmodel.h"
#include "document.h"

class HeatMapProvider : public QQuickImageProvider {
friend class HeatMapCalc;

public:
  HeatMapProvider();

  QImage requestImage(const QString &id, QSize *size,
                      const QSize &requestedSize) override;

protected:
  void setHeatMapImage(const QImage &heatMapImage);

private:
  QImage mLegend;
  QImage mHeatMapImage;

  void legend();
};


class HeatMapCalc : public QObject {
  Q_OBJECT
public:
  HeatMapCalc(HeatMapProvider *heatMapProvider, MeasurementModel *model, Document *document, QObject *parent = nullptr);

  void generateHeatMap();
signals:
  void heatMapReady();

private:
  HeatMapProvider *mHeatMapProvider;
  MeasurementModel *mModel;
  Document *mDocument;
};
