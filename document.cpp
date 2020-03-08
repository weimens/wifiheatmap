#include "document.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <quazip5/quazip.h>
#include <quazip5/quazipfile.h>

Document::Document(QObject *parent) : QObject(parent) {}

void Document::newDocument() {
  setMeasurements();
  setMapImageUrl(QUrl(":/A4_120dpi.png"));
  setNeedsSaving(false);
}

bool Document::save(QUrl fileUrl) {

  QuaZip zip(fileUrl.toLocalFile());
  if (!zip.open(QuaZip::Mode::mdCreate)) {
    return false;
  }

  QJsonObject root;
  QJsonArray data;
  for (auto mi : mMeasurements->items()) {
    QJsonObject point;
    point["x"] = mi.pos.x();
    point["y"] = mi.pos.y();
    QJsonArray scanInfo;
    for (auto s : mi.scan) {
      QJsonObject scanItem;
      scanItem["bssid"] = s.second.bssid;
      scanItem["ssid"] = s.second.ssid;
      scanItem["signal"] = s.second.signal;
      scanItem["freq"] = s.second.freq;
      scanItem["channel"] = s.second.channel;
      scanInfo.append(scanItem);
    }
    point["scanInfo"] = scanInfo;
    data.append(point);
  }
  root["data"] = data;

  QJsonDocument jdoc(root);

  QuaZipFile dataFile(&zip);
  dataFile.open(QIODevice::WriteOnly, QuaZipNewInfo("data.json"));
  dataFile.write(jdoc.toJson(QJsonDocument::Compact));
  dataFile.close();

  QuaZipFile mapfile(&zip);
  mapfile.open(QIODevice::WriteOnly, QuaZipNewInfo("mapimage.png"));
  mMapImage.save(&mapfile, "png");
  mapfile.close();

  zip.close();

  setNeedsSaving(false);
  return true;
}

void Document::load(QUrl fileUrl) {
  QuaZip zip(fileUrl.toLocalFile());
  if (!zip.open(QuaZip::Mode::mdUnzip)) {
    return;
  }
  setMeasurements();

  QuaZipFile file(&zip);
  for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
    file.open(QIODevice::ReadOnly);
    if (file.getActualFileName() == "data.json") {
      read(file.readAll());
    } else if (file.getActualFileName() == "mapimage.png") {
      QImage image;
      image.load(&file, "png");
      setMapImage(image);
    }
    file.close();
  }
  zip.close();

  setNeedsSaving(false);
}

Measurements *Document::measurements() const { return mMeasurements; }

void Document::read(QByteArray data) {
  QJsonDocument jdoc(QJsonDocument::fromJson(data));
  QJsonObject root = jdoc.object();

  if (root.contains("data") && root["data"].isArray()) {
    QJsonArray data = root["data"].toArray();
    for (int i = 0; i < data.size(); ++i) {
      QJsonObject point = data[i].toObject();
      if (point.contains("x") && point["x"].isDouble() && point.contains("y") &&
          point["y"].isDouble() && point.contains("scanInfo") &&
          point["scanInfo"].isArray()) {

        QJsonArray scanInfo = point["scanInfo"].toArray();
        std::map<QString, ScanInfo> scan;
        for (int j = 0; j < scanInfo.size(); ++j) {
          QJsonObject scanItem = scanInfo[j].toObject();
          if (scanItem.contains("bssid") && scanItem["bssid"].isString() &&
              scanItem.contains("ssid") && scanItem["ssid"].isString() &&
              scanItem.contains("signal") && scanItem["signal"].isDouble() &&
              scanItem.contains("freq") && scanItem["freq"].isDouble() &&
              scanItem.contains("channel") && scanItem["channel"].isDouble()) {
            ScanInfo item;
            item.bssid = scanItem["bssid"].toString();
            item.ssid = scanItem["ssid"].toString();
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
