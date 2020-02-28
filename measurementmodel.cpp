#include "measurementmodel.h"

MeasurementModel::MeasurementModel(TriggerScan *scanner, QObject *parent)
    : QAbstractListModel(parent), mInterfaceIndex(0), mScanner(scanner),
      mCurrentBss({}), mKownBssid({}), mList({}),
      mScanIndex(QPersistentModelIndex()) {
  connect(mScanner, &TriggerScan::scanFinished, this,
          &MeasurementModel::scanFinished);
  connect(mScanner, &TriggerScan::scanFailed, this,
          &MeasurementModel::scanFailed);
}

void MeasurementModel::setCurrentBSS(std::list<std::string> bss) {
  mCurrentBss = bss;
}

QHash<int, QByteArray> MeasurementModel::roleNames() const {
  return {
      {posRole, "pos"},
      {zRole, "z"},
      {stateRole, "state"},
  };
}

int MeasurementModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid())
    return 0;
  return mList.size();
}

bool MeasurementModel::setData(const QModelIndex &index, const QVariant &value,
                               int role) {
  if (!hasIndex(index.row(), index.column(), index.parent()) ||
      !value.isValid())
    return false;

  MeasurementItem &item = mList[index.row()];
  if (role == posRole)
    item.pos = value.value<QPoint>();
  else
    return false;

  emit dataChanged(index, index, {role});

  return true;
}

QVariant MeasurementModel::data(const QModelIndex &index, int role) const {
  if (!hasIndex(index.row(), index.column(), index.parent()))
    return {};

  const MeasurementItem &item = mList.at(index.row());
  if (role == posRole)
    return item.pos;
  if (role == zRole)
    return getMaxZ(item);
  if (role == stateRole) {
    return item.scan.size() > 0;
  }

  return {};
}

bool MeasurementModel::insertRows(int row, int count,
                                  const QModelIndex &parent) {

  if (count < 1 || row < 0 || row > rowCount(parent))
    return false;

  beginInsertRows(QModelIndex(), row, row + count - 1);

  for (int r = 0; r < count; ++r)
    mList.insert(row, MeasurementItem({}));

  endInsertRows();

  return true;
}

void MeasurementModel::measure(QPoint pos) {
  if (mScanner->trigger_scan(mInterfaceIndex)) {
    insertRows(rowCount(), 1);
    QModelIndex idx = index(rowCount() - 1);
    setData(idx, pos, posRole);
    mScanIndex = QPersistentModelIndex(idx);
  }
}

void MeasurementModel::append(MeasurementItem mi) {
  insertRows(rowCount(), 1);
  auto idx = index(rowCount() - 1);
  setData(idx, mi.pos, posRole);
  updateScan(idx, mi.scan);
}

void MeasurementModel::updateScan(
    QModelIndex idx, std::map<std::string, NetLink::scan_info> scan) {
  for (auto s : scan) {
    NetLink::scan_info scan_info = s.second;
    if (std::find(mKownBssid.begin(), mKownBssid.end(), scan_info.bssid) ==
        mKownBssid.end()) {
      mKownBssid.push_back(scan_info.bssid);
      emit bssAdded(scan_info.bssid.c_str(), scan_info.ssid.c_str(),
                    scan_info.freq, scan_info.channel);
    }
  }

  MeasurementItem &item = mList[idx.row()];
  item.scan = scan;
  emit dataChanged(idx, idx, {zRole, stateRole});
}

bool MeasurementModel::removeRows(int row, int count,
                                  const QModelIndex &parent) {
  if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
    return false;
  beginRemoveRows(QModelIndex(), row, row + count - 1);

  const auto it = mList.begin() + row;
  mList.erase(it, it + count);

  endRemoveRows();
  return true;
}

void MeasurementModel::remove(int row, int count) { removeRows(row, count); }

QList<MeasurementItem> MeasurementModel::getMeasurementItems() { return mList; }

void MeasurementModel::setInterfaceIndex(int index) { mInterfaceIndex = index; }

void MeasurementModel::bssChanged(QList<QString> bss) {
  mCurrentBss = {};
  for (auto a : bss) {
    mCurrentBss.push_back(a.toStdString());
  }
  for (int i = 0; i < rowCount(); ++i) {
    QModelIndex idx = index(i, 0);
    emit dataChanged(idx, idx, {zRole});
  }
  emit heatMapChanged();
}

void MeasurementModel::scanFinished() {
  if (!mScanIndex.isValid())
    return;

  NetLink::Nl80211 nl80211;
  NetLink::MessageScan msg(mInterfaceIndex);
  nl80211.sendMessageWait(&msg);
  const std::map<std::string, NetLink::scan_info> &scans = msg.getScan();
  if (scans.size() == 0) {
    remove(mScanIndex.row());
    return;
  }

  updateScan(mScanIndex, scans);
  emit heatMapChanged();
}

void MeasurementModel::scanFailed(int err) {
  // TODO: display message
  if (mScanIndex.isValid())
    remove(mScanIndex.row());
}

float MeasurementModel::getMaxZ(const MeasurementItem &item) const {
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
