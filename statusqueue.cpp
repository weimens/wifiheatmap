#include "statusqueue.h"

#include <QModelIndex>
#include <QTimer>

StatusQueue::StatusQueue(QObject *parent) : QAbstractListModel(parent) {}

int StatusQueue::rowCount(const QModelIndex &parent) const {
  return mMessages.size();
}

int StatusQueue::size() { return mMessages.size(); }

QVariant StatusQueue::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return {};
  return mMessages.at(index.row());
}

void StatusQueue::push(QString message) {
  emit beginInsertRows(QModelIndex(), mMessages.size(), mMessages.size());
  mMessages.enqueue(message);
  emit endInsertRows();
  emit sizeChanged();

  QTimer::singleShot(5000, this, &StatusQueue::pop);
}

void StatusQueue::pop() {
  emit beginRemoveRows(QModelIndex(), mMessages.size() - 1,
                       mMessages.size() - 1);
  mMessages.dequeue();
  emit endRemoveRows();

  for (int i = 0; i < mMessages.size(); ++i) { // FIXME: queue
    auto idx = index(i);
    emit dataChanged(idx, idx, {Qt::DisplayRole});
  }

  emit sizeChanged();
}
