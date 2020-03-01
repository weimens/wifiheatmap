#include "measurements.h"
#include <cmath>

Measurements::Measurements(QObject *parent)
    : QObject(parent), mCurrentBss({}), mKownBssid({}) {}

bool Measurements::setItemAt(int index, const MeasurementItem &item) {
  if (index < 0 || index >= mItems.size())
    return false;

  const MeasurementItem &oldItem = mItems.at(index);
  if (item.pos == oldItem.pos && item.scan.size() == oldItem.scan.size())
    return false; // FIXME: item.scan == oldItem.scan

  mItems[index] = item;

  emit ItemChanged();
  emit heatMapChanged();
  updateBss(item.scan);

  return true;
}

qreal Measurements::maxZAt(int index) const {
  if (index < 0 || index >= mItems.size())
    return NAN;

  const MeasurementItem item = mItems.at(index);
  float max_z = -INFINITY;
  for (std::string bss : mCurrentBss) {
    if (item.scan.find(bss) != item.scan.end())
      max_z = std::max(max_z, item.scan.at(bss).signal);
  }
  if (std::isinf(max_z)) {
    return NAN;
  }
  return max_z;
}

QPoint Measurements::posAt(int index) const {
  if (index < 0 || index >= mItems.size())
    return QPoint();
  return mItems.at(index).pos;
}

void Measurements::appendItem(MeasurementItem item) {
  emit preItemAppended();

  mItems.append(item);

  emit postItemAppended();
  updateBss(item.scan);
}

void Measurements::removeAt(int index) {
  if (index < 0 || index >= mItems.size())
    return;
  emit preItemRemoved(index);

  mItems.removeAt(index);

  emit postItemRemoved();
  emit heatMapChanged();
}

QVector<MeasurementItem> Measurements::items() const { return mItems; }

void Measurements::bssChanged(QList<QString> bss) {
  mCurrentBss = {};
  for (auto a : bss) {
    mCurrentBss.push_back(a.toStdString());
  }
  emit heatMapChanged();
}

void Measurements::updateBss(std::map<std::string, NetLink::scan_info> scan) {
  for (std::pair<std::string, NetLink::scan_info> s : scan) {
    NetLink::scan_info scan_info = s.second;
    if (std::find(mKownBssid.begin(), mKownBssid.end(), scan_info.bssid) ==
        mKownBssid.end()) {
      mKownBssid.push_back(scan_info.bssid);
      emit bssAdded(scan_info.bssid.c_str(), scan_info.ssid.c_str(),
                    scan_info.freq, scan_info.channel);
    }
  }
}
