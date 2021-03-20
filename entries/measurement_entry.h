#pragma once

#include "bss.h"
#include "measurement_type.h"

struct MeasurementEntry {
  Bss bss;
  double value;
  MeasurementType measurementType;

  MeasurementEntry(Bss bss, MeasurementType measurementType, double value)
      : bss(bss), measurementType(measurementType), value(value) {}
};
