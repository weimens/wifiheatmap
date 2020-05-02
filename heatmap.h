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

  Q_PROPERTY(double zmax READ zmax WRITE setZmax NOTIFY zmaxChanged)
  Q_PROPERTY(double zmin READ zmin WRITE setZmin NOTIFY zminChanged)

public:
  HeatMapCalc(HeatMapProvider *heatMapProvider, Document *document,
              QObject *parent = nullptr);

  void measurementsChanged(Measurements *measurements);

  void generateHeatMap();

  void setZmax(double max);
  void setZmin(double min);

  double zmax();
  double zmin();

signals:
  void heatMapReady();
  void zmaxChanged(long max);
  void zminChanged(long min);

private:
  HeatMapProvider *mHeatMapProvider;
  Document *mDocument;
  std::vector<std::vector<double>> heatmapZ;
  void generateImage();
  double mZmin{-80};
  double mZmax{-54};
};
