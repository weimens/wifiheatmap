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

class HeatMapLegend : public QObject {
  Q_OBJECT

  Q_PROPERTY(double zmax READ zmax WRITE setZmax NOTIFY zmaxChanged)
  Q_PROPERTY(double zmin READ zmin WRITE setZmin NOTIFY zminChanged)

  Q_PROPERTY(double zfrom READ zfrom NOTIFY zfromChanged)
  Q_PROPERTY(double zto READ zto NOTIFY ztoChanged)
  Q_PROPERTY(double zstepSize READ zstepSize NOTIFY zstepSizeChanged)

public:
  explicit HeatMapLegend(QObject *parent = nullptr);

  double zto();
  double zfrom();
  double zstepSize();
  Q_INVOKABLE QString zFormatter(double value);
  double flip();

  void setZmax(double max);
  void setZmin(double min);

  double zmax();
  double zmin();

public slots:
  void selectedTypeChanged(MeasurementType selectedType);

signals:
  void zfromChanged(double zfrom);
  void ztoChanged(double zto);
  void zstepSizeChanged(double zstepSize);
  void zmaxChanged(double max);
  void zminChanged(double min);

private:
  double mFrom{-100};
  double mTo{0};
  unsigned int mStepSize{1};
  QString mZunit{"dbm"};
  double mZmin{-80};
  double mZmax{-54};
  double mFlip{-1};
};

class HeatMapCalc : public QObject {
  Q_OBJECT

public:
  HeatMapCalc(HeatMapProvider *heatMapProvider, Document *document,
              HeatMapLegend *heatMapLegend, QObject *parent = nullptr);

  void measurementsChanged(Measurements *measurements);

  void generateHeatMap();

signals:
  void heatMapReady();

private:
  HeatMapProvider *mHeatMapProvider;
  Document *mDocument;
  HeatMapLegend *mHeatMapLegend;
  std::vector<std::vector<double>> heatmapZ;
  void generateImage();
};
