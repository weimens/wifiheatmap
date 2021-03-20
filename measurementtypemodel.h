#pragma once

#include "entries/measurement_type.h"
#include "measurements.h"
#include <QAbstractTableModel>

class MeasurementTypeModel : public QAbstractListModel {
  Q_OBJECT

  Q_PROPERTY(int currentIndex MEMBER mCurrentIndex NOTIFY currentIndexChanged);

public:
  explicit MeasurementTypeModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;

public slots:
  void measurementsChanged(Measurements *measurements);

private slots:
  void updateCurrentIndex();

signals:
  void selectedTypeChanged(MeasurementType selectedType);
  void currentIndexChanged();

private:
  Measurements *mMeasurements;
  MeasurementType selectedType;
  int mCurrentIndex;
};
