#include "document.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Document::Document(QObject *parent) : QObject(parent) {}

void Document::newDocument() {
  setMeasurements();
  setMapImageUrl(QUrl(":/A4_120dpi.png"));
  setNeedsSaving(false);
}

bool Document::save(QUrl fileUrl) {
  QFile file(fileUrl.toLocalFile());
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }

  QJsonObject root;
  QJsonArray data;
  for (auto mi : mMeasurements->items()) {
    QJsonObject point;
    point["x"] = mi.pos.x();
    point["y"] = mi.pos.y();
    QJsonArray nl80211_scan;
    for (auto s : mi.scan) {
      QJsonObject scanItem;
      scanItem["bssid"] = QString::fromStdString(s.second.bssid);
      scanItem["ssid"] = QString::fromStdString(s.second.ssid);
      scanItem["signal"] = s.second.signal;
      scanItem["freq"] = s.second.freq;
      scanItem["channel"] = s.second.channel;
      nl80211_scan.append(scanItem);
    }
    point["nl80211_scan"] = nl80211_scan;
    data.append(point);
  }
  root["data"] = data;

  QJsonDocument jdoc(root);
  file.write(jdoc.toJson(QJsonDocument::Compact));
  setNeedsSaving(false);
  return true;
}

void Document::load(QUrl fileUrl) {
  QFile file(fileUrl.toLocalFile());
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }
  setMeasurements();

  QJsonDocument jdoc(QJsonDocument::fromJson(file.readAll()));
  QJsonObject root = jdoc.object();

  if (root.contains("data") && root["data"].isArray()) {
    QJsonArray data = root["data"].toArray();
    for (int i = 0; i < data.size(); ++i) {
      QJsonObject point = data[i].toObject();
      if (point.contains("x") && point["x"].isDouble() && point.contains("y") &&
          point["y"].isDouble() && point.contains("nl80211_scan") &&
          point["nl80211_scan"].isArray()) {

        QJsonArray nl80211_scan = point["nl80211_scan"].toArray();
        std::map<std::string, NetLink::scan_info> scan;
        for (int j = 0; j < nl80211_scan.size(); ++j) {
          QJsonObject scanItem = nl80211_scan[j].toObject();
          if (scanItem.contains("bssid") && scanItem["bssid"].isString() &&
              scanItem.contains("ssid") && scanItem["ssid"].isString() &&
              scanItem.contains("signal") && scanItem["signal"].isDouble() &&
              scanItem.contains("freq") && scanItem["freq"].isDouble() &&
              scanItem.contains("channel") && scanItem["channel"].isDouble()) {
            NetLink::scan_info item;
            item.bssid = scanItem["bssid"].toString().toStdString();
            item.ssid = scanItem["ssid"].toString().toStdString();
            item.signal = scanItem["signal"].toDouble();
            item.freq = scanItem["freq"].toInt();
            item.channel = scanItem["channel"].toInt();
            scan[item.bssid] = item;
          }
        }
        MeasurementItem mi;
        mi.pos = QPoint(point["x"].toInt(), point["y"].toInt());
        mi.scan = scan;
        mMeasurements->appendItem(mi);
      }
    }
  }
  setNeedsSaving(false);
}

QImage Document::mapImage() const { return mMapImage; }

void Document::setMapImage(const QImage &mapImage) {
  mMapImage = mapImage;
  emit mapImageChanged();
}

// FIXME: is there a better way to reset Measurements
void Document::setMeasurements() {
  Measurements *oldMeasurements = nullptr;
  if (mMeasurements) {
    oldMeasurements = mMeasurements;
  }

  mMeasurements = new Measurements(this);

  if (mMeasurements) {
    connect(mMeasurements, &Measurements::postItemAppended, this,
            [=]() { setNeedsSaving(true); });
    connect(mMeasurements, &Measurements::postItemRemoved, this, [=]() {
      if (mMeasurements->items().size() == 0)
        setNeedsSaving(false);
      else
        setNeedsSaving(true);
    });
    connect(mMeasurements, &Measurements::ItemChanged, this,
            [=]() { setNeedsSaving(true); });
  }

  measurementsChanged(mMeasurements);
  if (oldMeasurements) {
    oldMeasurements->deleteLater();
  }
}

void Document::setNeedsSaving(bool value) {
  if (mNeedsSaving != value) {
    mNeedsSaving = value;
    emit needsSavingChanged(mNeedsSaving);
  }
}

void Document::setMapImageUrl(const QUrl &mapImageUrl) {
  setMapImage(QImage(mapImageUrl.path()));
}

bool Document::needsSaving() const { return mNeedsSaving; }
