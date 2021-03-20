#pragma once

#include "bss.h"
#include "measurement_type.h"

class MeasurementEntry {
public:
  Bss bss;
  double value;
  MeasurementType measurementType;

  MeasurementEntry() = default;
  MeasurementEntry(const MeasurementEntry &) = default;
  MeasurementEntry &operator=(const MeasurementEntry &) = default;

  MeasurementEntry(Bss bss, MeasurementType measurementType, double value)
      : bss(bss), value(value), measurementType(measurementType) {}
};
