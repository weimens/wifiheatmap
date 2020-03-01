#pragma once

#include <QImage>
#include <QQuickImageProvider>

#include "document.h"
#include "measurements.h"

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
  HeatMapCalc(HeatMapProvider *heatMapProvider, Document *document,
              QObject *parent = nullptr);

  void measurementsChanged(Measurements *measurements);

  void generateHeatMap();
signals:
  void heatMapReady();

private:
  HeatMapProvider *mHeatMapProvider;
  Document *mDocument;
};
