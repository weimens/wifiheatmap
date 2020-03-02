#include "measurementmodel.h"
#include "netlinkwrapper.h"

MeasurementModel::MeasurementModel(Document *document, TriggerScan *scanner,
                                   QObject *parent)
    : QAbstractListModel(parent), mInterfaceIndex(0), mScanner(scanner),
      mScanIndex(QPersistentModelIndex()), mMeasurements(nullptr) {

  connect(mScanner, &TriggerScan::scanFinished, this,
          &MeasurementModel::scanFinished);
  connect(mScanner, &TriggerScan::scanFailed, this,
          &MeasurementModel::scanFailed);
  connect(document, &Document::measurementsChanged, this,
          &MeasurementModel::setMeasurements);
  setMeasurements(document->measurements());
}

QHash<int, QByteArray> MeasurementModel::roleNames() const {
  return {
      {posRole, "pos"},
      {zRole, "z"},
      {stateRole, "state"},
  };
}

int MeasurementModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid() || !mMeasurements)
    return 0;

  return mMeasurements->items().size();
}

bool MeasurementModel::setData(const QModelIndex &index, const QVariant &value,
                               int role) {
  if (!index.isValid() || !mMeasurements || !value.isValid())
    return false;

  MeasurementItem item = mMeasurements->items().at(index.row());
  if (role == posRole)
    item.pos = value.value<QPoint>();
  else
    return false;

  if (mMeasurements->setItemAt(index.row(), item)) {
    emit dataChanged(index, index, {role});
    return true;
  }

  return false;
}

Qt::ItemFlags MeasurementModel::flags(const QModelIndex &index) const {
  if (!index.isValid())
    return Qt::NoItemFlags;

  return Qt::ItemIsEditable;
}

QVariant MeasurementModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || !mMeasurements)
    return {};

  const MeasurementItem item = mMeasurements->items().at(index.row());
  switch (role) {
  case posRole:
    return item.pos;
  case zRole:
    return mMeasurements->maxZAt(index.row());
  case stateRole:
    return item.scan.size() > 0;
  }

  return {};
}

bool MeasurementModel::measure(QPoint pos) {
  if (!mMeasurements || !mScanner)
    return false;
  if (mScanner->trigger_scan(mInterfaceIndex)) {
    mMeasurements->appendItem({pos, {}});
    mScanIndex = QPersistentModelIndex(index(rowCount() - 1));
    return true;
  }
  return false;
}

void MeasurementModel::remove(int row) {
  if (!mMeasurements)
    return;
  mMeasurements->removeAt(row);
}

void MeasurementModel::setInterfaceIndex(int index) { mInterfaceIndex = index; }

void MeasurementModel::scanFinished() {
  if (!mScanIndex.isValid() || !mMeasurements)
    return;

  NetLink::Nl80211 nl80211;
  NetLink::MessageScan msg(mInterfaceIndex);
  nl80211.sendMessageWait(&msg);
  const std::map<std::string, NetLink::scan_info> &scans = msg.getScan();
  if (scans.size() == 0) {
    mMeasurements->removeAt(mScanIndex.row());
    return;
  }

  MeasurementItem item = mMeasurements->items().at(mScanIndex.row());
  for (auto s : scans) {
    QString bssid = QString::fromStdString(s.first);
    item.scan[bssid] = ScanInfo();
    item.scan[bssid].bssid = QString::fromStdString(s.second.bssid);
    item.scan[bssid].ssid = QString::fromStdString(s.second.ssid);
    item.scan[bssid].lastSeen = s.second.last_seen;
    item.scan[bssid].freq = s.second.freq;
    item.scan[bssid].signal = s.second.signal;
    item.scan[bssid].channel = s.second.channel;
  }
  if (mMeasurements->setItemAt(mScanIndex.row(), item)) {
    emit dataChanged(mScanIndex, mScanIndex, {zRole, stateRole});
  }
}

void MeasurementModel::scanFailed(int err) {
  // TODO: display message
  if (mScanIndex.isValid())
    remove(mScanIndex.row());
}

Measurements *MeasurementModel::measurements() const { return mMeasurements; }

void MeasurementModel::setMeasurements(Measurements *measurements) {
  beginResetModel();

  if (measurements)
    measurements->disconnect(this);

  mMeasurements = measurements;

  if (mMeasurements) {
    connect(mMeasurements, &Measurements::preItemAppended, this, [=]() {
      const int index = mMeasurements->items().size();
      beginInsertRows(QModelIndex(), index, index);
    });
    connect(mMeasurements, &Measurements::postItemAppended, this,
            [=]() { endInsertRows(); });

    connect(mMeasurements, &Measurements::preItemRemoved, this,
            [=](int index) { beginRemoveRows(QModelIndex(), index, index); });
    connect(mMeasurements, &Measurements::postItemRemoved, this,
            [=]() { endRemoveRows(); });

    connect(mMeasurements, &Measurements::heatMapChanged, this, [=]() {
      for (int i = 0; i < rowCount(); ++i) {
        QModelIndex idx = index(i, 0);
        emit dataChanged(idx, idx, {zRole});
      }
    });
  }

  endResetModel();
}
