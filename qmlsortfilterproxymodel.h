#pragma once

#include <QSortFilterProxyModel>
class QmlSortFilterProxyModel : public QSortFilterProxyModel {
  Q_OBJECT

public:
  QmlSortFilterProxyModel(QObject *parent = nullptr);

  Q_INVOKABLE void sort(int column);

private:
  int last_column;
  Qt::SortOrder last_order;
};
