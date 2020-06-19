#include "measurementtypemodel.h"

MeasurementTypeModel::MeasurementTypeModel(QObject *parent)
    : QAbstractListModel(parent), mMeasurements(nullptr), mCurrentIndex(-1) {
  connect(this, &MeasurementTypeModel::currentIndexChanged, this,
          &MeasurementTypeModel::updateCurrentIndex);
}

int MeasurementTypeModel::rowCount(const QModelIndex &parent) const {
  return mMeasurements->measurementTypes().size();
}

QVariant MeasurementTypeModel::data(const QModelIndex &index, int role) const {
  if (role == Qt::DisplayRole) {
    auto measurementType = mMeasurements->measurementTypes().at(index.row());
    switch (measurementType) {
    case WiFiSignal:
      return "WiFi signal";
    case IperfRx:
      return "Iperf RX";
    case IperfTx:
      return "Iperf TX";
    case IperfRetransmits:
      return "Iperf retransmits";
    default:
      break;
    }
  }
  return {};
}

void MeasurementTypeModel::measurementsChanged(Measurements *measurements) {
  beginResetModel();

  mMeasurements = measurements;
  selectedType = WiFiSignal;

  if (mMeasurements) {
    connect(this, &MeasurementTypeModel::selectedTypeChanged, measurements,
            &Measurements::selectedTypeChanged);

    connect(mMeasurements, &Measurements::preTypeAppended, this,
            [=](int index, int count) {
              beginInsertRows(QModelIndex(), index, index + count - 1);
            });
    connect(mMeasurements, &Measurements::postTypeAppended, this, [=]() {
      endInsertRows();
      if (mMeasurements->measurementTypes().size() == 1) {
        mCurrentIndex = 0;
        emit currentIndexChanged();
      }
    });

    connect(mMeasurements, &Measurements::preTypeRemoved, this,
            [=](int index, int count) {
              beginRemoveRows(QModelIndex(), index, index + count - 1);
            });
    connect(mMeasurements, &Measurements::postTypeRemoved, this,
            [=]() { endRemoveRows(); });
  }

  endResetModel();
}

void MeasurementTypeModel::updateCurrentIndex() {
  auto idx = index(mCurrentIndex, 0);
  if (idx.isValid()) {
    auto measurementType = mMeasurements->measurementTypes().at(idx.row());
    emit selectedTypeChanged(measurementType);
  }
}
