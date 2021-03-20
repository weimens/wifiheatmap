#pragma once

#include "bss.h"
#include "measurement_type.h"
#include "position.h"

struct Measurement {
  Position pos;
  Bss bss;
  MeasurementType measurementType;
  double value;

  bool operator==(const Measurement &b) const {
    return this->bss == b.bss && this->pos == b.pos &&
           this->measurementType == b.measurementType;
  }
};
