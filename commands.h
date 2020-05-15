#pragma once

#include <QUndoCommand>

#include "measurements.h"

class AddMeasurementsAtPosition : public QUndoCommand {
public:
  AddMeasurementsAtPosition(Measurements *measurements, Position position,
                            QVector<MeasurementEntry> values);

  void redo();
  void undo();

private:
  Measurements *mMeasurements;
  Position mPosition;
  QVector<MeasurementEntry> mValues;
};

class RemovePosition : public QUndoCommand {
public:
  RemovePosition(Measurements *measurements, Position position);

  void redo();
  void undo();

private:
  Measurements *mMeasurements;
  QVector<Measurement> mMeasurement;
  Position mPosition;
};

class UpdatePosition : public QUndoCommand {
public:
  UpdatePosition(Measurements *measurements, Position oldPosition,
                 Position newPosition);

  void redo();
  void undo();

private:
  Measurements *mMeasurements;
  Position mOldPosition;
  Position mNewPosition;
};
