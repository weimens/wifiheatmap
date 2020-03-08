#pragma once

#include <QAbstractListModel>
#include <QPoint>

#include "document.h"
#include "measurements.h"

class MeasurementModel : public QAbstractListModel {
  Q_OBJECT
  Q_ENUMS(Roles)
  Q_PROPERTY(Measurements *measurements READ measurements WRITE setMeasurements)

public:
  enum Roles {
    posRole,
    zRole,
    stateRole,
  };

  MeasurementModel(Document *document, QObject *parent = nullptr);

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;

  Q_INVOKABLE void remove(int row);

  QList<MeasurementItem> getMeasurementItems();

  void setMeasurements(Measurements *measurements);

  Qt::ItemFlags flags(const QModelIndex &index) const override;

  Measurements *measurements() const;

public slots:
  void scanFinished(QList<ScanInfo> results);
  void scanFailed(int err);
  void scanStarted(QPoint pos);

private:
  QPersistentModelIndex mScanIndex;
  Measurements *mMeasurements;
};
