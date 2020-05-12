#pragma once

#include "measurements.h"
#include <QAbstractTableModel>

class BssModel : public QAbstractTableModel {
  Q_OBJECT

public:
  explicit BssModel(QObject *parent = nullptr);

  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override;

public slots:
  void measurementsChanged(Measurements *measurements);

signals:
  void selectedBssChanged(QVector<Bss> selectedBss);

private:
  Measurements *mMeasurements;
  QVector<Bss> selectedBss;
};
