#include "measurementmodel.h"

MeasurementModel::MeasurementModel(QObject *parent)
    : QAbstractListModel(parent), mScanIndex(QPersistentModelIndex()),
      mMeasurements(nullptr) {
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

void MeasurementModel::remove(int row) {
  if (!mMeasurements)
    return;
  mMeasurements->removeAt(row);
}

void MeasurementModel::scanFinished(QList<ScanInfo> results) {
  if (!mScanIndex.isValid() || !mMeasurements)
    return;

  MeasurementItem item = mMeasurements->items().at(mScanIndex.row());
  for (auto s : results) {
    item.scan[s.bssid] = s;
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

void MeasurementModel::scanStarted(QPoint pos) {
  mMeasurements->appendItem({pos, {}});
  mScanIndex = QPersistentModelIndex(index(rowCount() - 1));
}

Measurements *MeasurementModel::measurements() const { return mMeasurements; }

void MeasurementModel::measurementsChanged(Measurements *measurements) {
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
