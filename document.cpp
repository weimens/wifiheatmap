#include "document.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Document::Document(MeasurementModel *model, QObject *parent) : QObject(parent) {
  m_model = model;
}

void Document::save(QUrl fileUrl) {
  QFile file(fileUrl.toLocalFile());
  if (!file.open(QIODevice::WriteOnly)) {
    return;
  }

  QJsonObject root;
  QJsonArray data;
  for (auto mi : m_model->getMeasurementItems()) {
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
}

void Document::load(QUrl fileUrl) {
  QFile file(fileUrl.toLocalFile());
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

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
        m_model->append(mi);
      }
    }
  }
}
