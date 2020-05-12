#include "measurements.h"
#include <cmath>
#include <set>

Measurements::Measurements(QObject *parent)
    : QObject(parent), mMeasurements({}), mPositions({}), mBss({}),
      mCurrentBss({}) {}

const QVector<Measurement> Measurements::measurements() const {
  return mMeasurements;
}

const QVector<Position> Measurements::positions() const { return mPositions; }

const QVector<Bss> Measurements::bsss() const { return mBss; }

qreal Measurements::maxZAt(int index) const {
  if (index < 0 || index >= mPositions.size())
    return NAN;

  auto pos = mPositions.at(index);
  double max_z = -INFINITY;
  for (auto bss : mCurrentBss) {
    auto iter =
        std::find_if(mMeasurements.begin(), mMeasurements.end(),
                     [pos, bss](Measurement measurement) -> bool {
                       return measurement.pos == pos && measurement.bss == bss;
                     });
    if (iter != mMeasurements.end())
      max_z = std::max(max_z, iter->value);
  }
  if (std::isinf(max_z)) {
    return NAN;
  }
  return max_z;
}

int Measurements::countAt(int index) const {
  auto position = mPositions.at(index);
  return std::count_if(mMeasurements.begin(), mMeasurements.end(),
                       [position](Measurement measurement) -> bool {
                         return measurement.pos == position;
                       });
}

QVector<Measurement> Measurements::measurementsAt(Position position) const {
  QVector<Measurement> ret;
  auto pos_iter = std::find(mPositions.begin(), mPositions.end(), position);

  if (pos_iter != mPositions.end()) {
    for (auto measurement : mMeasurements) {
      if (measurement.pos == *pos_iter) {
        ret.push_back(measurement);
      }
    }
  }
  return ret;
}

void Measurements::newMeasurementsAtPosition(
    Position position, const QVector<QPair<Bss, double>> &values) {
  auto pos_iter = std::find(mPositions.begin(), mPositions.end(), position);

  if (pos_iter != mPositions.end()) {

    emit preMeasurementAppended(mMeasurements.size(), values.size());
    for (auto value : values) {
      auto bss_iter = std::find(mBss.begin(), mBss.end(), value.first);
      if (bss_iter == mBss.end()) {
        emit preBssAppended(mBss.size());
        mBss.push_back(value.first);
        emit postBssAppended();
        bss_iter = &(mBss.last());
      }

      mMeasurements.push_back(
          Measurement{*(pos_iter), *(bss_iter), value.second});
    }
    emit postMeasurementAppended();

    emit positionChanged(std::distance(mPositions.begin(), pos_iter));
    emit heatMapChanged();
  }
}

void Measurements::addPosition(Position position) {
  auto pos_iter = std::find_if(
      mPositions.begin(), mPositions.end(),
      [position](Position pos) -> bool { return pos == position; });

  if (pos_iter == mPositions.end()) {
    emit prePositionAppended(mPositions.size());
    mPositions.push_back(position);
    emit postPositionAppended();
  }
}

QVector<Measurement> Measurements::removePosition(Position position) {
  QVector<Measurement> measurementsRemoved;

  auto pos_iter = std::find_if(
      mPositions.begin(), mPositions.end(),
      [position](Position pos) -> bool { return pos == position; });
  if (pos_iter == mPositions.end()) {
    return measurementsRemoved;
  }

  auto iter = mMeasurements.begin();
  while (iter != mMeasurements.end()) {
    if (iter->pos == *(pos_iter)) {
      int i = std::distance(mMeasurements.begin(), iter);
      emit preMeasurementRemoved(i);
      measurementsRemoved.append(*iter);
      mMeasurements.removeAt(i);
      emit postMeasurementRemoved();
    } else {
      ++iter;
    }
  }

  auto bss_iter = mBss.begin();
  while (bss_iter != mBss.end()) {
    auto bss_count = std::count_if(mMeasurements.begin(), mMeasurements.end(),
                                   [bss_iter](Measurement measurement) -> bool {
                                     return measurement.bss == *bss_iter;
                                   });
    if (bss_count <= 0) {
      auto bss_index = std::distance(mBss.begin(), bss_iter);
      emit preBssRemoved(bss_index);
      mBss.removeAt(bss_index);
      emit postBssRemoved();
    } else {
      ++bss_iter;
    }
  }

  auto pos_index = std::distance(mPositions.begin(), pos_iter);
  emit prePositionRemoved(pos_index);
  mPositions.removeAt(pos_index);
  emit postPositionRemoved();

  if (measurementsRemoved.size() > 0)
    emit heatMapChanged();

  return measurementsRemoved;
}

void Measurements::updatePosition(Position oldPosition, Position newPosition) {
  auto pos_iter = std::find(mPositions.begin(), mPositions.end(), oldPosition);
  if (pos_iter != mPositions.end()) {
    for (auto iter = mMeasurements.begin(); iter != mMeasurements.end();
         ++iter) {
      if (iter->pos == *pos_iter) {
        iter->pos.pos = newPosition.pos;
      }
    }

    pos_iter->pos = newPosition.pos;
    emit positionChanged(std::distance(mPositions.begin(), pos_iter));
    emit heatMapChanged();
  }
}

void Measurements::selectedBssChanged(QVector<Bss> selectedBss) {
  mCurrentBss = {};
  for (auto a : selectedBss) {
    mCurrentBss.push_back(a);
  }
  emit heatMapChanged();
}
