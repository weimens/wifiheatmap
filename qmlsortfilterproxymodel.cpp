
#include "qmlsortfilterproxymodel.h"

QmlSortFilterProxyModel::QmlSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent), last_column(0),
      last_order(Qt::AscendingOrder) {
  QSortFilterProxyModel::sort(last_column, last_order);
}

Q_INVOKABLE void QmlSortFilterProxyModel::sort(int column) {
  if (last_column == column) {
    last_order = (last_order == Qt::AscendingOrder) ? Qt::DescendingOrder
                                                    : Qt::AscendingOrder;
  } else {
    last_order = Qt::AscendingOrder;
  }
  QSortFilterProxyModel::sort(column, last_order);
  last_column = column;
}
