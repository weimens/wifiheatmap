#pragma once

#include "measurementmodel.h"
#include <QObject>
#include <QUrl>

class Document : public QObject {
  Q_OBJECT

public:
  Document(MeasurementModel *model, QObject *parent = nullptr);

  Q_INVOKABLE void save(QUrl fileUrl);
  Q_INVOKABLE void load(QUrl fileUrl);

private:
  MeasurementModel *m_model;
};
