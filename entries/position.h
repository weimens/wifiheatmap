#pragma once

#include <QPoint>

struct Position {
  QPoint pos;

public:
  bool operator==(const Position &b) const { return this->pos == b.pos; }
};
