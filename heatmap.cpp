#include "heatmap.h"

#include "delaunator-header-only.hpp"
#include <algorithm>
#include <cmath>

struct find_simplex_result {
  bool found;
  size_t pos;
  double A;
  double B;
  double C;
};

/*
 * Find simplex that contains point x,y by brute-force.
 */
struct find_simplex_result
find_simplex(const std::vector<double> &transform, const double &x,
             const double &y,
             const double eps = std::numeric_limits<double>::epsilon()) {
  for (std::size_t k = 0; k < transform.size(); k += 6) {
    // barycentric coordinates
    const double Bx = transform[k + 4];
    const double By = transform[k + 5];
    const double A = transform[k + 0] * (x - Bx) + transform[k + 1] * (y - By);
    const double C = transform[k + 2] * (x - Bx) + transform[k + 3] * (y - By);
    const double B = 1 - A - C;

    // point is inside simplex
    if (A - eps > 0 && B - eps > 0 && C - eps > 0) {
      return find_simplex_result{true, k / 2, A, B, C};
    }
  }
  return find_simplex_result{false, 0, 0, 0, 0};
}

static const QVector<QRgb> colors = {
    qRgba(0, 0, 0, 0),         qRgba(165, 0, 38, 255),
    qRgba(173, 8, 38, 255),    qRgba(181, 15, 38, 255),
    qRgba(189, 23, 38, 255),   qRgba(197, 30, 39, 255),
    qRgba(205, 38, 39, 255),   qRgba(213, 46, 39, 255),
    qRgba(218, 55, 42, 255),   qRgba(223, 64, 47, 255),
    qRgba(227, 74, 51, 255),   qRgba(232, 84, 55, 255),
    qRgba(237, 94, 60, 255),   qRgba(241, 103, 64, 255),
    qRgba(245, 113, 69, 255),  qRgba(246, 123, 74, 255),
    qRgba(247, 134, 78, 255),  qRgba(249, 144, 83, 255),
    qRgba(250, 154, 88, 255),  qRgba(252, 165, 93, 255),
    qRgba(253, 175, 98, 255),  qRgba(253, 183, 104, 255),
    qRgba(253, 191, 111, 255), qRgba(253, 199, 118, 255),
    qRgba(254, 207, 124, 255), qRgba(254, 214, 131, 255),
    qRgba(254, 222, 138, 255), qRgba(254, 228, 146, 255),
    qRgba(254, 233, 154, 255), qRgba(254, 238, 162, 255),
    qRgba(255, 243, 170, 255), qRgba(255, 248, 179, 255),
    qRgba(255, 253, 187, 255), qRgba(252, 254, 187, 255),
    qRgba(246, 251, 179, 255), qRgba(240, 249, 170, 255),
    qRgba(234, 246, 162, 255), qRgba(228, 244, 154, 255),
    qRgba(222, 241, 146, 255), qRgba(215, 238, 138, 255),
    qRgba(207, 235, 133, 255), qRgba(199, 231, 127, 255),
    qRgba(191, 228, 122, 255), qRgba(183, 224, 117, 255),
    qRgba(175, 221, 112, 255), qRgba(167, 217, 107, 255),
    qRgba(157, 213, 105, 255), qRgba(147, 209, 104, 255),
    qRgba(137, 204, 103, 255), qRgba(126, 200, 102, 255),
    qRgba(116, 195, 101, 255), qRgba(106, 191, 99, 255),
    qRgba(95, 185, 97, 255),   qRgba(83, 180, 94, 255),
    qRgba(71, 174, 91, 255),   qRgba(59, 168, 88, 255),
    qRgba(47, 162, 85, 255),   qRgba(34, 156, 82, 255),
    qRgba(25, 150, 79, 255),   qRgba(21, 142, 75, 255),
    qRgba(17, 134, 71, 255),   qRgba(12, 127, 67, 255),
    qRgba(8, 119, 63, 255),    qRgba(4, 112, 59, 255),
    qRgba(0, 104, 55, 255),
};

HeatMapProvider::HeatMapProvider()
    : QQuickImageProvider(QQuickImageProvider::Image) {
  mHeatMapImage = QImage(1, 1, QImage::Format_Indexed8);
  mHeatMapImage.setColorCount(1);
  mHeatMapImage.setColor(0, qRgba(0, 0, 0, 0));
  mHeatMapImage.setPixel(0, 0, 0);
  legend();
}

QImage HeatMapProvider::requestImage(const QString &id, QSize *size,
                                     const QSize &requestedSize) {
  QImage image;
  if (id.startsWith("legend")) {
    image = mLegend;
  } else {
    image = mHeatMapImage;
  }
  size->setWidth(image.width());
  size->setHeight(image.height());

  return image;
}

void HeatMapProvider::legend() {
  // first entry in colors is tranparent
  mLegend = QImage(colors.size() - 1, 1, QImage::Format_Indexed8);
  mLegend.setColorCount(colors.size() - 1);
  for (int i = 1; i < colors.size(); ++i) {
    mLegend.setColor(i - 1, colors[i]);
  }

  for (int i = 0; i < mLegend.colorCount(); ++i) {
    mLegend.setPixel(i, 0, i);
  }
}

void HeatMapProvider::setHeatMapImage(const QImage &heatMapImage) {
  mHeatMapImage = heatMapImage;
}

HeatMapLegend::HeatMapLegend(QObject *parent) : QObject(parent) {}

double HeatMapLegend::zfrom() { return mFrom; }

double HeatMapLegend::zto() { return mTo; }

double HeatMapLegend::zstepSize() { return mStepSize; }

void HeatMapLegend::setZmax(double max) {
  if (mZmax == max)
    return;
  mZmax = max;
  emit zmaxChanged(max);
}

void HeatMapLegend::setZmin(double min) {
  if (mZmin == min)
    return;
  mZmin = min;
  emit zminChanged(min);
}

double HeatMapLegend::zmax() { return mZmax; }

double HeatMapLegend::zmin() { return mZmin; }

void HeatMapLegend::selectedTypeChanged(MeasurementType selectedType) {
  double from = 0;
  double to = 1;
  unsigned int stepSize = 1;

  double min = 0;
  double max = 1;

  switch (selectedType) {
  case WiFiSignal:
    from = -100;
    to = 0;
    stepSize = 1;
    mZunit = "dbm";
    min = -80;
    max = -54;
    mFlip = -1;
    break;
  case IperfRx:
  case IperfTx:
    stepSize = 1000 * 1000;
    from = 0;
    to = 1000 * stepSize;
    mZunit = "MBit/s";
    min = 11 * 1000 * 1000;
    max = 97 * 1000 * 1000;
    mFlip = -1;
    break;
  case IperfRetransmits:
    stepSize = 1;
    from = 0;
    to = 500;
    mZunit = "";
    min = 10;
    max = 0;
    mFlip = 1;
    break;
  }

  if (mStepSize != stepSize) {
    mStepSize = stepSize;
    emit zstepSizeChanged(mStepSize);
  }

  if (mFrom > from) {
    mFrom = from;
    emit zfromChanged(mFrom);
  }
  if (mTo < to) {
    mTo = to;
    emit ztoChanged(mTo);
  }

  setZmin(min);
  setZmax(max);

  if (mFrom < from) {
    mFrom = from;
    emit zfromChanged(mFrom);
  }
  if (mTo > to) {
    mTo = to;
    emit ztoChanged(mTo);
  }
}

QString HeatMapLegend::zFormatter(double value) {
  return QString::number(value / mStepSize) + "\u2009" + mZunit;
}

double HeatMapLegend::flip() { return mFlip; }

HeatMapCalc::HeatMapCalc(HeatMapProvider *heatMapProvider, Document *document,
                         HeatMapLegend *heatMapLegend, QObject *parent)
    : QObject(parent), mHeatMapProvider(heatMapProvider), mDocument(document),
      mHeatMapLegend(heatMapLegend) {
  connect(mHeatMapLegend, &HeatMapLegend::zmaxChanged, this,
          [this]() { generateImage(); });
  connect(mHeatMapLegend, &HeatMapLegend::zminChanged, this,
          [this]() { generateImage(); });

  connect(document, &Document::mapImageChanged, this,
          &HeatMapCalc::generateHeatMap);
  connect(document, &Document::measurementsChanged, this,
          &HeatMapCalc::measurementsChanged);
  measurementsChanged(document->measurements());
}

void HeatMapCalc::measurementsChanged(Measurements *measurements) {
  if (!measurements)
    return;

  connect(measurements, &Measurements::heatMapChanged, this,
          [=]() { generateHeatMap(); });
  generateHeatMap();
}

void HeatMapCalc::generateHeatMap() {
  if (!(mDocument && mDocument->measurements()))
    return;

  int maxsize = 512;
  int nx;
  int ny;

  int w = mDocument->mapImage().size().width();
  int h = mDocument->mapImage().size().height();
  qreal ratio = 1.0;
  if (h != 0) {
    ratio = w / static_cast<qreal>(h);
  }

  if (ratio >= 1.0) {
    nx = maxsize;
    ny = static_cast<int>(maxsize * ratio);
  } else {
    nx = static_cast<int>(maxsize * ratio);
    ny = maxsize;
  }

  std::vector<double> coords;
  std::vector<double> values;
  for (int i = 0; i < mDocument->measurements()->positions().size(); ++i) {
    qreal z = mDocument->measurements()->maxZAt(i);
    if (!std::isnan(z)) {
      QPoint pos = mDocument->measurements()->positions().at(i).pos;
      coords.push_back(pos.x());
      coords.push_back(pos.y());
      values.push_back(z);
    }
  }

  heatmapZ = std::vector<std::vector<double>>(nx, std::vector<double>(ny, NAN));

  if (coords.size() >= 3 * 2) {
    delaunator::Delaunator d(coords);
    std::vector<double> transform(d.triangles.size() * 2);

    for (std::size_t i = 0; i < d.triangles.size(); i += 3) {
      const double detinv = 1 / ((coords[2 * d.triangles[i + 1] + 1] -
                                  coords[2 * d.triangles[i + 2] + 1]) *
                                     (coords[2 * d.triangles[i + 0] + 0] -
                                      coords[2 * d.triangles[i + 2] + 0]) +
                                 (coords[2 * d.triangles[i + 2] + 0] -
                                  coords[2 * d.triangles[i + 1] + 0]) *
                                     (coords[2 * d.triangles[i + 0] + 1] -
                                      coords[2 * d.triangles[i + 2] + 1]));

      const std::size_t j = i * 2;

      // transform from cartesian x to barycentric coordinates
      // c = Tinv (x-r)

      // Tinv
      transform[j] = (coords[2 * d.triangles[i + 1] + 1] -
                      coords[2 * d.triangles[i + 2] + 1]) *
                     detinv;
      transform[j + 1] = (coords[2 * d.triangles[i + 2] + 0] -
                          coords[2 * d.triangles[i + 1] + 0]) *
                         detinv;
      transform[j + 2] = (coords[2 * d.triangles[i + 0] + 1] -
                          coords[2 * d.triangles[i + 1] + 1]) *
                         detinv;
      transform[j + 3] = (coords[2 * d.triangles[i + 1] + 0] -
                          coords[2 * d.triangles[i + 0] + 0]) *
                         detinv;
      // r
      transform[j + 4] = coords[2 * d.triangles[i + 1] + 0];
      transform[j + 5] = coords[2 * d.triangles[i + 1] + 1];
    }

    for (int i = 0; i < nx; ++i) {
      for (int j = 0; j < ny; ++j) {
        double x = i / static_cast<double>(nx) * w;
        double y = j / static_cast<double>(ny) * h;

        struct find_simplex_result found = find_simplex(transform, x, y);

        if (!found.found) {
          continue;
        }

        // linear barycentric interpolation
        heatmapZ[i][j] = values[d.triangles[found.pos + 0]] * found.A +
                         values[d.triangles[found.pos + 1]] * found.B +
                         values[d.triangles[found.pos + 2]] * found.C;
      }
    }
  }
  generateImage();
}

void HeatMapCalc::generateImage() {
  int nx = heatmapZ.size();
  if (nx <= 0)
    return;
  int ny = heatmapZ[0].size();
  if (nx <= 0)
    return;

  QImage image = QImage(nx, ny, QImage::Format_Indexed8);
  int colorCount = colors.size() - 1;
  image.setColorCount(colorCount + 1);
  for (int i = 0; i < colors.size(); ++i) {
    image.setColor(i, colors[i]);
  }

  for (int i = 0; i < nx; ++i) {
    for (int j = 0; j < ny; ++j) {
      double z = heatmapZ[i][j];
      if (!std::isnan(z)) {
        if (mHeatMapLegend->flip() < 0) {
          if (z < mHeatMapLegend->zmin())
            z = mHeatMapLegend->zmin();
          if (z > mHeatMapLegend->zmax())
            z = mHeatMapLegend->zmax();
        } else {
          if (z < mHeatMapLegend->zmax())
            z = mHeatMapLegend->zmax();
          if (z > mHeatMapLegend->zmin())
            z = mHeatMapLegend->zmin();
        }
        z = 1 - mHeatMapLegend->flip() *
                    floor((z - mHeatMapLegend->zmin()) /
                          abs(mHeatMapLegend->zmin() - mHeatMapLegend->zmax()) *
                          (colorCount - 1));
        image.setPixel(i, j, std::max(z, 0.));
      } else {
        image.setPixel(i, j, 0);
      }
    }
  }

  mHeatMapProvider->setHeatMapImage(image);
  emit heatMapReady();
}
