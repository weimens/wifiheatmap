#include "bssmodel.h"

BssModel::BssModel(QObject *parent)
    : QAbstractTableModel(parent), mMeasurements(nullptr) {}

QHash<int, QByteArray> BssModel::roleNames() const {
  return {{Qt::DisplayRole, "display"}, {Qt::CheckStateRole, "checkstate"}};
}

void BssModel::measurementsChanged(Measurements *measurements) {
  beginResetModel();

  if (measurements)
    measurements->disconnect(this);

  mMeasurements = measurements;
  selectedBss = {};

  if (mMeasurements) {
    connect(this, &BssModel::selectedBssChanged, measurements,
            &Measurements::selectedBssChanged);

    connect(mMeasurements, &Measurements::preBssAppended, this,
            [=](int index, int count) {
              beginInsertRows(QModelIndex(), index, index + count - 1);
            });
    connect(mMeasurements, &Measurements::postBssAppended, this,
            [=]() { endInsertRows(); });

    connect(mMeasurements, &Measurements::preBssRemoved, this,
            [=](int index, int count) {
              beginRemoveRows(QModelIndex(), index, index + count - 1);
            });
    connect(mMeasurements, &Measurements::postBssRemoved, this,
            [=]() { endRemoveRows(); });
  }

  endResetModel();
}

int BssModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid() || !mMeasurements)
    return 0;

  return mMeasurements->bsss().size();
}

QVariant BssModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || !mMeasurements)
    return {};

  auto bss = mMeasurements->bsss().at(index.row());

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case 0:
      return bss.ssid;
    case 1:
      return bss.bssid;
    case 2:
      return bss.freq;
    case 3:
      return bss.channel;
    default:
      break;
    }
  }
  if (role == Qt::CheckStateRole && index.column() == 4) {
    auto iter = std::find(selectedBss.begin(), selectedBss.end(), bss);
    if (iter != selectedBss.end()) {
      return Qt::Checked;
    } else {
      return Qt::Unchecked;
    }
  }

  return {};
}

QVariant BssModel::headerData(int section, Qt::Orientation orientation,
                              int role) const {
  switch (section) {
  case 0:
    return "ssid";
  case 1:
    return "bss";
  case 2:
    return "freq";
  case 3:
    return "ch";
  default:
    return "";
  }
}

int BssModel::columnCount(const QModelIndex &parent) const { return 5; }

bool BssModel::setData(const QModelIndex &index, const QVariant &value,
                       int role) {
  if (!index.isValid() || !mMeasurements || !value.isValid())
    return false;
  if (role == Qt::CheckStateRole && index.column() == 4) {
    auto checkedNew = value.toBool();

    auto bss = mMeasurements->bsss().at(index.row());
    auto iter = std::find(selectedBss.begin(), selectedBss.end(), bss);
    auto checkedOld = iter != selectedBss.end();

    if (checkedOld && !checkedNew) {
      selectedBss.erase(iter);
    } else if (!checkedOld && checkedNew) {
      selectedBss.push_back(bss);
    } else {
      return false;
    }
    emit selectedBssChanged(selectedBss);
    emit dataChanged(index, index, {role});
    return true;
  }
  return false;
}
