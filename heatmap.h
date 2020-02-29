#pragma once

#include <QImage>
#include <QQuickImageProvider>

#include "measurementmodel.h"

class HeatMapProvider : public QQuickImageProvider {
public:
  HeatMapProvider(MeasurementModel *model);

  QImage requestImage(const QString &id, QSize *size,
                      const QSize &requestedSize) override;

private:
  MeasurementModel *mModel;
  QImage mLegend;

  void legend();
  QImage updateHeatMapPlot(qreal ratio, QSize size);
};
