#pragma once

#include <QObject>
#include <QPoint>
#include <QVector>

#include "entries/bss.h"
#include "entries/measurement.h"
#include "entries/measurement_entry.h"
#include "entries/measurement_type.h"
#include "entries/position.h"

class Measurements : public QObject {
  Q_OBJECT

public:
  explicit Measurements(QObject *parent = nullptr);

  const QVector<Measurement> measurements() const;
  const QVector<Position> positions() const;
  const QVector<Bss> bsss() const;
  const QVector<MeasurementType> measurementTypes() const;

  qreal maxZAt(int index) const;
  int countAt(int index) const;
  QVector<Measurement> measurementsAt(Position position) const;

  void newMeasurementsAtPosition(Position position,
                                 const QVector<MeasurementEntry> &values);

  void addPosition(Position position);
  QVector<Measurement> removePosition(Position position);
  void updatePosition(Position oldPosition, Position newPosition);

signals:
  void preMeasurementAppended(int index, int count = 1);
  void postMeasurementAppended();
  void prePositionAppended(int index, int count = 1);
  void postPositionAppended();
  void preBssAppended(int index, int count = 1);
  void postBssAppended();
  void preTypeAppended(int index, int count = 1);
  void postTypeAppended();

  void preMeasurementRemoved(int index, int count = 1);
  void postMeasurementRemoved();
  void prePositionRemoved(int index, int count = 1);
  void postPositionRemoved();
  void preBssRemoved(int index, int count = 1);
  void postBssRemoved();
  void preTypeRemoved(int index, int count = 1);
  void postTypeRemoved();

  void positionChanged(int index);

  void heatMapChanged();

public slots:
  void selectedBssChanged(QVector<Bss> selectedBss);
  void selectedTypeChanged(MeasurementType selectedType);

private:
  QVector<Measurement> mMeasurements;
  QVector<Position> mPositions;
  QVector<Bss> mBss;
  QVector<MeasurementType> mMeasurementTypes;

  QVector<Bss> mCurrentBss;
  MeasurementType mCurrentMeasurementType{WiFiSignal};
};
