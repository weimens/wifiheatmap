#include "measurementmodel.h"
#include "commands.h"

MeasurementModel::MeasurementModel(QUndoStack *undoStack, QObject *parent)
    : QAbstractListModel(parent), mScanIndex(QPersistentModelIndex()),
      mMeasurements(nullptr), mUndoStack(undoStack) {}

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

  return mMeasurements->positions().size();
}

bool MeasurementModel::setData(const QModelIndex &index, const QVariant &value,
                               int role) {
  if (!index.isValid() || !mMeasurements || !value.isValid())
    return false;

  auto pos = mMeasurements->positions().at(index.row());
  if (role == posRole) {
    auto pos_new = Position{value.value<QPoint>()};

    auto command = new UpdatePosition{mMeasurements, pos, pos_new};
    mUndoStack->push(command);
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

  auto position = mMeasurements->positions().at(index.row());
  switch (role) {
  case posRole:
    return position.pos;
  case zRole:
    return mMeasurements->maxZAt(index.row());
  case stateRole:
    return mMeasurements->countAt(index.row()) > 0;
  }

  return {};
}

void MeasurementModel::remove(int row) {
  if (!mMeasurements)
    return;

  auto pos = mMeasurements->positions().at(row);
  auto command = new RemovePosition{mMeasurements, pos};
  mUndoStack->push(command);
}

void MeasurementModel::scanFinished(QVector<MeasurementEntry> results) {
  if (!mScanIndex.isValid() || !mMeasurements)
    return;

  auto position = mMeasurements->positions().at(mScanIndex.row());
  auto command =
      new AddMeasurementsAtPosition{mMeasurements, position, results};
  mUndoStack->push(command);

  emit dataChanged(mScanIndex, mScanIndex, {zRole, stateRole});
  mScanIndex = QPersistentModelIndex();
}

void MeasurementModel::scanFailed(int err) {
  // TODO: display message
  auto position = mMeasurements->positions().at(mScanIndex.row());
  mMeasurements->removePosition(position);
  mScanIndex = QPersistentModelIndex();
}

void MeasurementModel::scanStarted(QPoint pos) {
  mMeasurements->addPosition(Position{pos});
  mScanIndex = QPersistentModelIndex(index(rowCount() - 1));
}

Measurements *MeasurementModel::measurements() const { return mMeasurements; }

void MeasurementModel::measurementsChanged(Measurements *measurements) {
  beginResetModel();

  if (measurements)
    measurements->disconnect(this);

  mMeasurements = measurements;

  if (mMeasurements) {
    connect(mMeasurements, &Measurements::prePositionAppended, this,
            [=](int index, int count) {
              beginInsertRows(QModelIndex(), index, index + count - 1);
            });
    connect(mMeasurements, &Measurements::postPositionAppended, this,
            [=]() { endInsertRows(); });

    connect(mMeasurements, &Measurements::prePositionRemoved, this,
            [=](int index, int count) {
              beginRemoveRows(QModelIndex(), index, index + count - 1);
            });
    connect(mMeasurements, &Measurements::postPositionRemoved, this,
            [=]() { endRemoveRows(); });

    connect(mMeasurements, &Measurements::heatMapChanged, this, [=]() {
      for (int i = 0; i < rowCount(); ++i) {
        QModelIndex idx = index(i, 0);
        emit dataChanged(idx, idx, {zRole});
      }
    });

    connect(mMeasurements, &Measurements::positionChanged, this, [=](int row) {
      QModelIndex idx = index(row, 0);
      emit dataChanged(idx, idx, {zRole, stateRole, posRole});
    });
  }

  endResetModel();
}
