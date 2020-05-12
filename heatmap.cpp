#include "heatmap.h"

#include <algorithm>

//#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Interpolation_traits_2.h>
#include <CGAL/interpolation_functions.h>
#include <CGAL/natural_neighbor_coordinates_2.h>

// typedef CGAL::Exact_predicates_exact_constructions_kernel K;
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_2<K> Delaunay_triangulation;
typedef CGAL::Interpolation_traits_2<K> Traits;
typedef K::FT Coord_type;
typedef K::Point_2 Point;

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

HeatMapCalc::HeatMapCalc(HeatMapProvider *heatMapProvider, Document *document,
                         QObject *parent)
    : QObject(parent), mHeatMapProvider(heatMapProvider), mDocument(document) {
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

  Delaunay_triangulation T;
  typedef std::map<Point, Coord_type, K::Less_xy_2> Coord_map;
  typedef CGAL::Data_access<Coord_map> Value_access;
  Coord_map value_function;

  int maxsize = 60;
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

  for (int i = 0; i < mDocument->measurements()->positions().size(); ++i) {
    qreal z = mDocument->measurements()->maxZAt(i);
    if (!std::isnan(z)) {
      QPoint pos = mDocument->measurements()->positions().at(i).pos;
      value_function.insert({Point(pos.x(), pos.y()), z});
    }
  }

  for (auto coord : value_function) {
    T.insert(coord.first);
  }

  heatmapZ = std::vector<std::vector<double>>(nx, std::vector<double>(ny, NAN));

  if (value_function.size() >= 3) {
    double x, y;
    for (int i = 0; i < nx; ++i) {
      for (int j = 0; j < ny; ++j) {
        x = i / static_cast<double>(nx) * w;
        y = j / static_cast<double>(ny) * h;

        std::vector<std::pair<Point, Coord_type>> coords;
        Point p = Point(x, y);

        auto res = CGAL::natural_neighbor_coordinates_2(
            T, p, std::back_inserter(coords));
        Coord_type norm = res.second;
        bool success = res.third;
        if (success) {
          Coord_type res = CGAL::linear_interpolation(
              coords.begin(), coords.end(), norm, Value_access(value_function));
          heatmapZ[i][j] = CGAL::to_double(res);
        }
      }
    }
  }
  generateImage();
}

void HeatMapCalc::setZmax(double max) {
  if (mZmax == max)
    return;
  mZmax = max;
  emit zmaxChanged(max);
  generateImage();
}

void HeatMapCalc::setZmin(double min) {
  if (mZmin == min)
    return;
  mZmin = min;
  emit zminChanged(min);
  generateImage();
}

double HeatMapCalc::zmax() { return mZmax; }

double HeatMapCalc::zmin() { return mZmin; }

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
      if (!isnan(z)) {
        if (z < mZmin)
          z = mZmin;
        if (z > mZmax)
          z = mZmax;
        z = 1 - floor((z - mZmin) / (mZmin - mZmax) * (colorCount - 1));
        image.setPixel(i, j, std::max(z, 0.));
      } else {
        image.setPixel(i, j, 0);
      }
    }
  }

  mHeatMapProvider->setHeatMapImage(image);
  emit heatMapReady();
}
