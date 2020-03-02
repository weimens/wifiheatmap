#pragma once

#include <QStandardItem>
#include <QStandardItemModel>

class InterfaceItem : public QStandardItem {
public:
  enum Roles {
    indexRole = Qt::UserRole + 6,
    nameRole = Qt::UserRole + 7,
  };

  InterfaceItem(unsigned char index, QString name);

  int type() const override;
};

class InterfaceModel : public QStandardItemModel {
  Q_OBJECT
  Q_PROPERTY(int currentIndex MEMBER mCurrentIndex NOTIFY currrentIndexChanged);
  Q_PROPERTY(int currentInterface MEMBER mCurrentInterface NOTIFY
                 currentInterfaceChanged)

public:
  InterfaceModel(QObject *parent = nullptr);

  void append(unsigned char index, QString name);

  void loadInterfaces();
signals:
  void currrentIndexChanged();
  void currentInterfaceChanged(int currentInterface);

private slots:
  void updateCurrentInterfaceIndex();

private:
  int mCurrentIndex;
  int mCurrentInterface;
};
