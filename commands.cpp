#include "commands.h"

AddMeasurementsAtPosition::AddMeasurementsAtPosition(
    Measurements *measurements, Position position,
    QVector<QPair<Bss, double>> values)
    : mMeasurements(measurements), mPosition(position), mValues(values) {}

void AddMeasurementsAtPosition::redo() {
  mMeasurements->addPosition(mPosition);
  mMeasurements->newMeasurementsAtPosition(mPosition, mValues);
}

void AddMeasurementsAtPosition::undo() {
  mMeasurements->removePosition(mPosition); // FIXME:
}

RemovePosition::RemovePosition(Measurements *measurements, Position position)
    : mMeasurements(measurements), mPosition(position) {}

void RemovePosition::redo() {
  mMeasurement = mMeasurements->removePosition(mPosition);
}

void RemovePosition::undo() {
  QVector<QPair<Bss, double>> values;
  for (auto measurement : mMeasurement) {
    values.append(QPair<Bss, double>{measurement.bss, measurement.value});
  }
  mMeasurements->addPosition(mPosition);
  mMeasurements->newMeasurementsAtPosition(mPosition, values);
}

UpdatePosition::UpdatePosition(Measurements *measurements, Position oldPosition,
                               Position newPosition)
    : mMeasurements(measurements), mOldPosition(oldPosition),
      mNewPosition(newPosition) {}

void UpdatePosition::redo() {
  mMeasurements->updatePosition(mOldPosition, mNewPosition);
}

void UpdatePosition::undo() {
  mMeasurements->updatePosition(mNewPosition, mOldPosition);
}
