#include "measurementmodel.h"

MeasurementModel::MeasurementModel(TriggerScan *scanner, QObject *parent)
    : QAbstractListModel(parent), interface_index(0), m_scanner(scanner),
      current_bss({}), kown_bssid({}), m_list({}),
      m_scan_index(QPersistentModelIndex()) {
  connect(m_scanner, &TriggerScan::scanFinished, this,
          &MeasurementModel::scanFinished);
  connect(m_scanner, &TriggerScan::scanFailed, this,
          &MeasurementModel::scanFailed);
}

void MeasurementModel::setCurrentBSS(std::list<std::string> bss) {
  current_bss = bss;
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
  return m_list.size();
}

bool MeasurementModel::setData(const QModelIndex &index, const QVariant &value,
                               int role) {
  if (!hasIndex(index.row(), index.column(), index.parent()) ||
      !value.isValid())
    return false;

  MeasurementItem &item = m_list[index.row()];
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

  const MeasurementItem &item = m_list.at(index.row());
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
    m_list.insert(row, MeasurementItem({}));

  endInsertRows();

  return true;
}

void MeasurementModel::measure(QPoint pos) {
  if (m_scanner->trigger_scan(interface_index)) {
    insertRows(rowCount(), 1);
    QModelIndex idx = index(rowCount() - 1);
    setData(idx, pos, posRole);
    m_scan_index = QPersistentModelIndex(idx);
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
    if (std::find(kown_bssid.begin(), kown_bssid.end(), scan_info.bssid) ==
        kown_bssid.end()) {
      kown_bssid.push_back(scan_info.bssid);
      emit bssAdded(scan_info.bssid.c_str(), scan_info.ssid.c_str(),
                    scan_info.freq, scan_info.channel);
    }
  }

  MeasurementItem &item = m_list[idx.row()];
  item.scan = scan;
  emit dataChanged(idx, idx, {zRole, stateRole});
}

bool MeasurementModel::removeRows(int row, int count,
                                  const QModelIndex &parent) {
  if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
    return false;
  beginRemoveRows(QModelIndex(), row, row + count - 1);

  const auto it = m_list.begin() + row;
  m_list.erase(it, it + count);

  endRemoveRows();
  return true;
}

void MeasurementModel::remove(int row, int count) { removeRows(row, count); }

QList<MeasurementItem> MeasurementModel::getMeasurementItems() {
  return m_list;
}

void MeasurementModel::setInterfaceIndex(int index) { interface_index = index; }

void MeasurementModel::bssChanged(QList<QString> bss) {
  current_bss = {};
  for (auto a : bss) {
    current_bss.push_back(a.toStdString());
  }
  for (int i = 0; i < rowCount(); ++i) {
    QModelIndex idx = index(i, 0);
    emit dataChanged(idx, idx, {zRole});
  }
  emit heatMapChanged();
}

void MeasurementModel::scanFinished() {
  if (!m_scan_index.isValid())
    return;

  NetLink::Nl80211 nl80211;
  NetLink::MessageScan msg(interface_index);
  nl80211.sendMessageWait(&msg);
  const std::map<std::string, NetLink::scan_info> &scans = msg.getScan();
  if (scans.size() == 0) {
    remove(m_scan_index.row());
    return;
  }

  updateScan(m_scan_index, scans);
  emit heatMapChanged();
}

void MeasurementModel::scanFailed(int err) {
  // TODO: display message
  if (m_scan_index.isValid())
    remove(m_scan_index.row());
}

float MeasurementModel::getMaxZ(const MeasurementItem &item) const {
  float max_z = -INFINITY;
  for (std::string bss : current_bss) {
    if (item.scan.find(bss) != item.scan.end())
      max_z = std::max(max_z, item.scan.at(bss).signal);
  }
  if (std::isinf(max_z)) {
    return NAN;
  }
  return max_z;
}
