#pragma once

#include <QAbstractListModel>
#include <QPoint>
#include <QUndoStack>

#include "entries.h"
#include "measurements.h"

class MeasurementModel : public QAbstractListModel {
  Q_OBJECT
  Q_ENUMS(Roles)

public:
  enum Roles {
    posRole,
    zRole,
    stateRole,
  };

  MeasurementModel(QUndoStack *undoStack, QObject *parent = nullptr);

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;

  Q_INVOKABLE void remove(int row);

  void measurementsChanged(Measurements *measurements);

  Qt::ItemFlags flags(const QModelIndex &index) const override;

  Measurements *measurements() const;

public slots:
  void scanFinished(QVector<MeasurementEntry> results);
  void scanFailed(int err);
  void scanStarted(QPoint pos);

private:
  QPersistentModelIndex mScanIndex;
  Measurements *mMeasurements;
  QUndoStack *mUndoStack;
};
