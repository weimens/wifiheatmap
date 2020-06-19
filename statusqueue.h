#pragma once

#include <QAbstractListModel>
#include <QQueue>
#include <QString>

class StatusQueue : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int size READ size NOTIFY sizeChanged)

public:
  explicit StatusQueue(QObject *parent = nullptr);
  int rowCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  void push(QString message);
  int size();

signals:
  void sizeChanged();

private:
  void pop();
  QQueue<QString> mMessages{};
};
