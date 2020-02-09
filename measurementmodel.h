#pragma once

#include <QAbstractListModel>
#include <QPoint>

struct MeasurementItem {
  QPoint pos;
  qreal z;
};

class MeasurementModel : public QAbstractListModel {
  Q_OBJECT
  Q_ENUMS(Roles)

public:
  enum Roles {
    posRole,
    zRole,
  };

  using QAbstractListModel::QAbstractListModel;

  QHash<int, QByteArray> roleNames() const override{
    return {
        {posRole, "pos"},
        {zRole, "z"},
    };
  }

  int rowCount(const QModelIndex &parent = QModelIndex()) const override {
    if (parent.isValid())
      return 0;
    return m_list.size();
  }

  bool setData(const QModelIndex &index, const QVariant &value,
               int role) override {
    if (!hasIndex(index.row(), index.column(), index.parent()) ||
        !value.isValid())
      return false;

    MeasurementItem &item = m_list[index.row()];
    if (role == posRole)
      item.pos = value.value<QPoint>();
    else if (role == zRole)
      item.z = value.toReal();
    else
      return false;

    emit dataChanged(index, index, {role});

    return true;
  }

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override {
    if (!hasIndex(index.row(), index.column(), index.parent()))
      return {};

    const MeasurementItem &item = m_list.at(index.row());
    if (role == posRole)
      return item.pos;
    if (role == zRole)
      return item.z;

    return {};
  }

  bool insertRows(int row, int count,
                  const QModelIndex &parent = QModelIndex()) override {

    if (count < 1 || row < 0 || row > rowCount(parent))
      return false;

    beginInsertRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
      m_list.insert(row, MeasurementItem({QPoint(10, 10), -4}));

    endInsertRows();

    return true;
  }

  void append(MeasurementItem item) {
    insertRows(rowCount(), 1);
    setData(index(rowCount() - 1), item.pos, posRole);
    setData(index(rowCount() - 1), item.z, zRole);
  }

  Q_INVOKABLE void append(QVariantMap vl) {
    if (vl.contains("pos") && vl.contains("z")) {
      insertRows(rowCount(), 1);
      setData(index(rowCount() - 1), vl["pos"], posRole);
      setData(index(rowCount() - 1), vl["z"], zRole);
    }
  }

  bool removeRows(int row, int count,
                  const QModelIndex &parent = QModelIndex()) override {
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
      return false;
    beginRemoveRows(QModelIndex(), row, row + count - 1);

    const auto it = m_list.begin() + row;
    m_list.erase(it, it + count);

    endRemoveRows();
    return true;
  }

  Q_INVOKABLE void remove(int row, int count = 1) { removeRows(row, count); }

  int size() { return m_list.size(); }

  MeasurementItem get(int row) { return m_list.at(row); }

private:
  QList<MeasurementItem> m_list;
};
